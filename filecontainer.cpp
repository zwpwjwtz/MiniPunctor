#include "filecontainer.h"
#include <QFile>
#include <QTextStream>


FileContainer::FileContainer()
{
    setFileType(PlainText);
}

FileErrorNumber FileContainer::open(QString path,
                                    TimeLine& recordList,
                                    bool forceOpen)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return openFail;

    QTextStream stream(&file);
    QString tempRecord;
    TimeTick tick;

    tempRecord = stream.read(header.length());
    if (tempRecord != header && !forceOpen)
        return headerError;
    recordList.clear();
    while(!stream.atEnd())
    {
        tempRecord = stream.readLine();
        if (!tempRecord.isEmpty())
        {
            tick.startTime = TimeLine::stringToTimeStamp(tempRecord,
                                                         timeFormat);
            tick.endTime = tick.startTime;
            recordList.appendItem(tick);
        }
    }
    tempRecord = stream.read(footer.length());
    if (tempRecord != footer && !forceOpen)
        return footerError;

    file.close();
    filePath = path;
    return fileOK;
}

FileErrorNumber FileContainer::save(TimeLine &recordList)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return openFail;

    int count = recordList.count();
    QString tempRecord;
    QString tempTime;

    file.write(header.toLocal8Bit());
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
            tempRecord.replace(PUNCTOR_FILE_FORMAT_VAR_TIME1, tempTime);
        }
        tempRecord.replace(PUNCTOR_FILE_FORMAT_VAR_CONTENT,
                           recordList[i].content);

        file.write(tempRecord.append(recordSeperator).toLocal8Bit());
    }
    file.write(footer.toLocal8Bit());

    file.close();
    return fileOK;
}

FileErrorNumber FileContainer::saveAs(TimeLine &recordList, QString path)
{
    QString oldPath = filePath;
    filePath = path;
    FileErrorNumber err = save(recordList);
    if (err != fileOK)
    {
        filePath = oldPath;
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

FileType FileContainer::getFileType()
{
    return _fileType;
}

void FileContainer::setFileType(FileType file_type)
{
    switch(file_type)
    {
        case PlainText:
            header = footer = "";
            recordFormat = PUNCTOR_FILE_FORMAT_PLAIN_RECORD;
            timeFormat = PUNCTOR_FILE_FORMAT_PLAIN_TIME;
            recordSeperator = PUNCTOR_FILE_FORMAT_PLAIN_SEP;
        default:
            header = footer = "";
            recordFormat = PUNCTOR_FILE_FORMAT_PLAIN_RECORD;
            timeFormat = PUNCTOR_FILE_FORMAT_PLAIN_TIME;
            recordSeperator = PUNCTOR_FILE_FORMAT_PLAIN_SEP;
    }
    _fileType = file_type;
}

QString FileContainer::getTimeFormat()
{
    return timeFormat;
}
