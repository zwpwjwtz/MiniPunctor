#ifndef FILECONTAINER_H
#define FILECONTAINER_H

#include "timeline.h"
#include "file_format.h"

enum FileType
{
    PlainText = 0,
    Customized = 255,
};

enum FileErrorNumber
{
    fileOK = 0,
    openFail = 1,
    readFail = 2,
    writeFail = 3,
    headerError = 4,
    footerError = 5,
};

class FileContainer
{
public:
    FileContainer();
    FileErrorNumber open(QString path, TimeLine &recordList, bool forceOpen);
    FileErrorNumber save(TimeLine &recordList);
    FileErrorNumber saveAs(TimeLine &recordList, QString path);
    void close();
    QString getFilePath();
    FileType getFileType();
    void setFileType(FileType file_type);
    QString getTimeFormat();

private:
    QString filePath;
    FileType _fileType;
    QString header;
    QString footer;
    QString recordSeperator;
    QString recordFormat;
    QString timeFormat;
};

#endif // FILECONTAINER_H
