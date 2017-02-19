#include "filecontainer.h"

#define PUNCTOR_FILE_BOM_UTF8 "\xEF\xBB\xBF"
#define PUNCTOR_FILE_BOM_UTF16BE "\xFE\xFF"
#define PUNCTOR_FILE_BOM_UTF16LE "\xFF\xFE"
#define PUNCTOR_FILE_PROBE_BLOCK_LEN 64


FileContainer::FileContainer()
{
    varPlaceHolder[0] = PUNCTOR_FILE_FORMAT_VAR_ID;
    varPlaceHolder[1] = PUNCTOR_FILE_FORMAT_VAR_TIME1;
    varPlaceHolder[2] = PUNCTOR_FILE_FORMAT_VAR_TIME2;
    varPlaceHolder[3] = PUNCTOR_FILE_FORMAT_VAR_CONTENT;

    setFileType(PlainText);
}

FileContainer::FileErrorNumber FileContainer::open(QString path,
                                                   TimeLine& recordList,
                                                   bool forceOpen)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return openFail;

    TimeTick tick;
    setFileType(guessFileType(path));

    header = file.read(seekFirstRecord(file));
    recordList.clear();
    while(!file.atEnd())
    {
        tick = recordToTimeTick(readUntil(file, recordSeperator));
        if (tick.startTime < 0 && !forceOpen)
        {
            file.close();
            return recordError;
        }
        recordList.appendItem(tick);
    }    
    footer = readUntil(file, "");

    file.close();
    filePath = path;
    return fileOK;
}

FileContainer::FileErrorNumber FileContainer::save(TimeLine &recordList)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
        return openFail;

    int count = recordList.count();
    QString tempRecord;
    QString tempTime;

    file.write(header);
    for (int i=0; i<count; i++)
    {
        tempRecord = recordFormat;
        tempRecord.replace(PUNCTOR_FILE_FORMAT_VAR_ID, recordList[i].id);
        tempTime = TimeLine::timeStampToString(recordList[i].startTime,
                                               timeFormat);
        tempRecord.replace(PUNCTOR_FILE_FORMAT_VAR_TIME1, tempTime);
        if (tempRecord.contains(PUNCTOR_FILE_FORMAT_VAR_TIME2))
        {
            tempTime = TimeLine::timeStampToString(recordList[i].endTime,
                                                   timeFormat);
            tempRecord.replace(PUNCTOR_FILE_FORMAT_VAR_TIME2, tempTime);
        }
        tempRecord.replace(PUNCTOR_FILE_FORMAT_VAR_CONTENT,
                           recordList[i].content);

        file.write(tempRecord.append(recordSeperator).toLocal8Bit());
    }
    file.write(footer);

    file.close();
    return fileOK;
}

FileContainer::FileErrorNumber FileContainer::saveAs(TimeLine &recordList,
                                                     QString path)
{
    QString oldPath = filePath;
    FileType oldType = _fileType;
    filePath = path;
    setFileType(guessFileType(path));
    FileErrorNumber err = save(recordList);
    if (err != fileOK)
    {
        filePath = oldPath;
        if (_fileType != oldType)
            setFileType(oldType);
        return err;
    }
    else
        return fileOK;
}

void FileContainer::close()
{
    filePath.clear();
}

QString FileContainer::getFilePath()
{
    return filePath;
}

FileContainer::FileType FileContainer::getFileType()
{
    return _fileType;
}

void FileContainer::setFileType(FileType file_type)
{
    switch(file_type)
    {
        case LRC:
            recordFormat = PUNCTOR_FILE_FORMAT_LRC_RECORD;
            timeFormat = PUNCTOR_FILE_FORMAT_LRC_TIME;
            recordSeperator = PUNCTOR_FILE_FORMAT_LRC_SEP;
            break;
        case SRT:
            recordFormat = PUNCTOR_FILE_FORMAT_SRT_RECORD;
            timeFormat = PUNCTOR_FILE_FORMAT_SRT_TIME;
            recordSeperator = PUNCTOR_FILE_FORMAT_SRT_SEP;
            break;
        case PlainText:
        default:
            recordFormat = PUNCTOR_FILE_FORMAT_PLAIN_RECORD;
            timeFormat = PUNCTOR_FILE_FORMAT_PLAIN_TIME;
            recordSeperator = PUNCTOR_FILE_FORMAT_PLAIN_SEP;
    }

    int varPos[PUNCTOR_FILE_FORMAT_MAX_VAR];
    int p = 0;
    int i = 0, j, varIndex;
    while (p < recordFormat.length())
    {
        varIndex = 0;
        for (j=0; j<4; j++)
        {
            varPos[j] = recordFormat.indexOf(varPlaceHolder[j], p);
            if (varPos[j] >= 0 &&
                    (varPos[j] < varPos[varIndex] || varPos[varIndex] < 0))
                varIndex = j;
        }
        if (varPos[varIndex] < 0)
            break;
        varSeperator[i] = recordFormat.mid(p, varPos[varIndex] - p);
        varOrder[i] = varIndex;
        p = varPos[varIndex] + varPlaceHolder[varIndex].length();
        i++;
    }
    varSeperator[i] = recordFormat.mid(p);
    varOrder[i] = -1;

    _fileType = file_type;
}

QString FileContainer::getTimeFormat()
{
    return timeFormat;
}

TimeTick FileContainer::recordToTimeTick(QString str)
{
    TimeTick tick;
    tick.startTime = tick.endTime = -1;
    int i = 0, p1 = 0, p2;
    QString tempVar;

    while (p1 < str.length() && varOrder[i] >= 0)
    {
        p1 += varSeperator[i].length();
        if (varSeperator[i + 1].isEmpty())
            p2 = str.length();
        else
            p2 = str.indexOf(varSeperator[i + 1], p1);
        tempVar = str.mid(p1, p2 - p1);

        switch (varOrder[i])
        {
            case 0:
                tick.id = tempVar;
                break;
            case 1:
                tick.startTime = TimeLine::stringToTimeStamp(tempVar,
                                                             timeFormat);
                break;
            case 2:
                tick.endTime = TimeLine::stringToTimeStamp(tempVar,
                                                             timeFormat);
                break;
            case 3:
            default:
                tick.content = tempVar;
        }

        p1 = p2;
        i++;
    }
    if (tick.endTime < 0)
        tick.endTime = tick.startTime;

    return tick;
}

QByteArray FileContainer::readUntil(QFile& file, const QString& splitter)
{
    int p, paddingLength;
    QByteArray buffer, content;

    paddingLength = splitter.length() - 1;
    while (!file.atEnd())
    {
        buffer = file.read(PUNCTOR_FILE_PROBE_BLOCK_LEN);
        p = buffer.indexOf(splitter);
        if (p >= 0)
        {
            file.seek(file.pos() - buffer.length() + p + paddingLength + 1);
            buffer.truncate(p);
            content.append(buffer);
            break;
        }
        else
        {
            buffer.truncate(buffer.length() - paddingLength);
            content.append(buffer);
        }

        if (file.atEnd())
            break;
        file.seek(file.pos() - paddingLength);
    }
    return content;
}

qint64 FileContainer::seekFirstRecord(QFile& file)
{
    int i, paddingLength = 0;
    qint64 oldPos, p = 0;
    QByteArray buffer;
    QString firstTimeVar;
    bool hasTimeVar = false;

    for (i=0; i<PUNCTOR_FILE_FORMAT_MAX_VAR; i++)
    {
        firstTimeVar = varPlaceHolder[varOrder[i]];
        if (firstTimeVar == PUNCTOR_FILE_FORMAT_VAR_TIME1 ||
            firstTimeVar == PUNCTOR_FILE_FORMAT_VAR_TIME2)
        {
            hasTimeVar = true;
            paddingLength = firstTimeVar.length() - 1;
            break;
        }
    }

    oldPos = file.pos();
    file.seek(0);
    while (!file.atEnd())
    {
        buffer = file.read(PUNCTOR_FILE_PROBE_BLOCK_LEN);
        if (hasTimeVar && i == 0)
        {
            p = seekTimeString(buffer, timeFormat);
            if (p < 0)
            {
                file.seek(file.pos() - paddingLength);
                continue;
            }
            else
                p -= varSeperator[i].length();
        }
        p += file.pos()
           + getBomLength(buffer)
           - buffer.length();
        break;
    }
    file.seek(oldPos);
    return p;
}

int FileContainer::seekTimeString(const QByteArray& str, const QString& format)
{
    int i, length, formatLength;
    bool found;

    found = false;
    length = str.length();
    formatLength = format.length();
    for (i=0; i < length; i++)
    {
        if (str[i] >= 0x30 && str[i] <= 0x39)
        {
            if (TimeLine::stringToTimeStamp(str.mid(i, formatLength),
                                            format) >= 0)
            {
                found = true;
                break;
            }
        }
    }
    if (found)
        return i;
    else
        return -1;
}

int FileContainer::getBomLength(QByteArray& str)
{
    int length = 0;
    if (str.indexOf(PUNCTOR_FILE_BOM_UTF8) >= 0)
        length += ::strlen(PUNCTOR_FILE_BOM_UTF8);
    if (str.indexOf(PUNCTOR_FILE_BOM_UTF16BE) >= 0)
        length += ::strlen(PUNCTOR_FILE_BOM_UTF16BE);
    if (str.indexOf(PUNCTOR_FILE_BOM_UTF16LE) >= 0)
        length += ::strlen(PUNCTOR_FILE_BOM_UTF16LE);
    return length;
}

FileContainer::FileType FileContainer::guessFileType(QString filePath)
{
    FileType tempType;
    QString suffix(filePath.mid(filePath.lastIndexOf('.') + 1).toLower());
    if (suffix == "lrc")
        tempType = LRC;
    else if (suffix == "srt")
        tempType = SRT;
    else
        tempType = PlainText;
    return tempType;
}
