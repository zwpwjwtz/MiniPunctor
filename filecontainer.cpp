#include "filecontainer.h"
#include "file_format.h"

#define PUNCTOR_FILE_BOM_UTF8 "\xEF\xBB\xBF"
#define PUNCTOR_FILE_BOM_UTF16BE "\xFE\xFF"
#define PUNCTOR_FILE_BOM_UTF16LE "\xFF\xFE"
#define PUNCTOR_FILE_PROBE_BLOCK_LEN 64


FileContainer::FileContainer()
{
    varPlaceHolder[0] = PUNCTOR_FILE_REC_VAR_ID;
    varPlaceHolder[1] = PUNCTOR_FILE_REC_VAR_TIME1;
    varPlaceHolder[2] = PUNCTOR_FILE_REC_VAR_TIME2;
    varPlaceHolder[3] = PUNCTOR_FILE_REC_VAR_CONTENT;

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

    if (bodyBegin.isEmpty())
        header = file.read(seekFirstRecord(file));
    else
    {
        header = readUntil(file, bodyBegin);
        header.append(bodyBegin).append(readUntil(file, recordSeparator));
    }
    footer.clear();

    qint64 p = 0;
    bool reachBodyEnd = false;
    recordList.clear();
    while(!(file.atEnd() || reachBodyEnd))
    {
        p = file.pos();
        tick = recordToTimeTick(readUntil(file, recordSeparator));
        if (file.atEnd() && !bodyEnd.isEmpty())
        {
            // Go back to the previous position,
            // and retry with another separator (BODYEND)
            file.seek(p);
            tick = recordToTimeTick(readUntil(file, bodyEnd));
            footer.append(bodyEnd);
            reachBodyEnd = true;
        }

        if (tick.startTime < 0)
        {
            if (tick.content.trimmed().isEmpty())
            {
                // Skip empty record
                continue;
            }
            else if (!forceOpen)
            {
                // Stop pharsing
                file.close();
                return recordError;
            }
        }
        recordList.appendItem(tick);
    }

    footer.append(file.readAll());

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

    if (header.isEmpty())
        file.write(defaultHeader);
    else
        file.write(header);
    for (int i=0; i<count; i++)
    {
        tempRecord = recordFormat;
        tempRecord.replace(PUNCTOR_FILE_REC_VAR_ID, recordList[i].id);
        tempTime = TimeLine::timeStampToString(recordList[i].startTime,
                                               timeFormat);
        tempRecord.replace(PUNCTOR_FILE_REC_VAR_TIME1, tempTime);
        if (tempRecord.contains(PUNCTOR_FILE_REC_VAR_TIME2))
        {
            tempTime = TimeLine::timeStampToString(recordList[i].endTime,
                                                   timeFormat);
            tempRecord.replace(PUNCTOR_FILE_REC_VAR_TIME2, tempTime);
        }
        tempRecord.replace(PUNCTOR_FILE_REC_VAR_CONTENT,
                           recordList[i].content);

        switch (recSepPos)
        {
            case PUNCTOR_FILE_REC_SEP_BEFORE:
                tempRecord.prepend(recordSeparator);
                break;
            case PUNCTOR_FILE_REC_SEP_AFTER:
                tempRecord.append(recordSeparator);
                break;
            default:
                if (i < count - 1)
                    tempRecord.append(recordSeparator);
        }

        file.write(tempRecord.toLocal8Bit());
    }
    if (footer.isEmpty())
        file.write(defaultFooter);
    else
        file.write(footer);

    file.close();
    return fileOK;
}

FileContainer::FileErrorNumber FileContainer::saveAs(TimeLine &recordList,
                                                     QString path)
{
    QString oldPath = filePath;
    FileType oldType = _fileType;
    QByteArray oldHeader = header;
    QByteArray oldFooter = footer;
    filePath = path;

    setFileType(guessFileType(path));
    if (_fileType != oldType)
    {
        header.clear();
        footer.clear();
    }

    FileErrorNumber err = save(recordList);
    if (err != fileOK)
    {
        filePath = oldPath;
        if (_fileType != oldType)
        {
            setFileType(oldType);
            header = oldHeader;
            footer = oldFooter;
        }
        return err;
    }
    else
        return fileOK;
}

void FileContainer::close()
{
    filePath.clear();
    setFileType(PlainText);
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
    defaultHeader.clear();
    defaultFooter.clear();
    bodyBegin.clear();
    bodyEnd.clear();

    switch(file_type)
    {
        case LRC:
            recordFormat = PUNCTOR_FILE_FORMAT_LRC_RECORD;
            timeFormat = PUNCTOR_FILE_FORMAT_LRC_TIME;
            recordSeparator = PUNCTOR_FILE_FORMAT_LRC_SEP;
            recSepPos = PUNCTOR_FILE_FORMAT_LRC_SEPPOS;
            break;
        case SRT:
            recordFormat = PUNCTOR_FILE_FORMAT_SRT_RECORD;
            timeFormat = PUNCTOR_FILE_FORMAT_SRT_TIME;
            recordSeparator = PUNCTOR_FILE_FORMAT_SRT_SEP;
            recSepPos = PUNCTOR_FILE_FORMAT_SRT_SEPPOS;
            break;
        case SMI:
            defaultHeader = PUNCTOR_FILE_FORMAT_SMI_HEADER;
            defaultFooter = PUNCTOR_FILE_FORMAT_SMI_FOOTER;
            bodyBegin = PUNCTOR_FILE_FORMAT_SMI_BODYBEGIN;
            bodyEnd = PUNCTOR_FILE_FORMAT_SMI_BODYEND;
            recSepPos = PUNCTOR_FILE_FORMAT_SMI_SEPPOS;
            recordFormat = PUNCTOR_FILE_FORMAT_SMI_RECORD;
            timeFormat = PUNCTOR_FILE_FORMAT_SMI_TIME;
            recordSeparator = PUNCTOR_FILE_FORMAT_SMI_SEP;
            break;
        case SSA:
            bodyBegin = PUNCTOR_FILE_FORMAT_SSA_BODYBEGIN;
            recSepPos = PUNCTOR_FILE_FORMAT_SSA_SEPPOS;
            recordFormat = PUNCTOR_FILE_FORMAT_SSA_RECORD;
            timeFormat = PUNCTOR_FILE_FORMAT_SSA_TIME;
            recordSeparator = PUNCTOR_FILE_FORMAT_SSA_SEP;
            break;
        case PlainText:
        default:
            recSepPos = PUNCTOR_FILE_FORMAT_PLAIN_SEPPOS;
            recordFormat = PUNCTOR_FILE_FORMAT_PLAIN_RECORD;
            timeFormat = PUNCTOR_FILE_FORMAT_PLAIN_TIME;
            recordSeparator = PUNCTOR_FILE_FORMAT_PLAIN_SEP;
    }

    int varPos[PUNCTOR_FILE_REC_MAX_VAR];
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
            p2 = str.indexOf(varSeperator[i + 1], p1, Qt::CaseInsensitive);
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
        p = buffer.indexOf(splitter, Qt::CaseInsensitive);
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

    for (i=0; i<PUNCTOR_FILE_REC_MAX_VAR; i++)
    {
        firstTimeVar = varPlaceHolder[varOrder[i]];
        if (firstTimeVar == PUNCTOR_FILE_REC_VAR_TIME1 ||
            firstTimeVar == PUNCTOR_FILE_REC_VAR_TIME2)
        {
            hasTimeVar = true;
            paddingLength = firstTimeVar.length() - 1;
            break;
        }
    }

    oldPos = file.pos();
    file.seek(0);
    while (true)
    {
        buffer = file.read(PUNCTOR_FILE_PROBE_BLOCK_LEN);
        if (hasTimeVar && i == 0)
        {
            p = seekTimeString(buffer, timeFormat);
            if (p < 0)
            {
                if (file.atEnd())
                {
                    p = file.size();
                    break;
                }
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
    else if (suffix == "smi")
        tempType = SMI;
    else if (suffix == "ssa")
        tempType = SSA;
    else
        tempType = PlainText;
    return tempType;
}
