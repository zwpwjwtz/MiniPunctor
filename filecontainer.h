#ifndef FILECONTAINER_H
#define FILECONTAINER_H

#include "timeline.h"
#include <QFile>

#define PUNCTOR_FILE_REC_MAX_VAR 32

class FileContainer
{
public:
    enum FileType
    {
        PlainText = 0,
        LRC = 1,
        SRT = 2,
        SMI = 3,
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
        recordError = 6
    };

    FileContainer();
    FileErrorNumber open(QString path, TimeLine &recordList, bool forceOpen);
    FileErrorNumber save(TimeLine &recordList);
    FileErrorNumber saveAs(TimeLine &recordList, QString path);
    void close();
    QString getFilePath();
    FileType getFileType();
    void setFileType(FileType file_type);
    QString getTimeFormat();
    TimeTick recordToTimeTick(QString str);
    static FileType guessFileType(QString filePath);

private:
    QString filePath;
    FileType _fileType;
    QByteArray header, footer;
    QByteArray bodyBegin, bodyEnd;
    int recSepPos;
    QString recordSeparator;
    QString recordFormat;
    QString timeFormat;
    QString varPlaceHolder[PUNCTOR_FILE_REC_MAX_VAR];
    int varOrder[PUNCTOR_FILE_REC_MAX_VAR + 1];
    QString varSeperator[PUNCTOR_FILE_REC_MAX_VAR + 1];

    static QByteArray readUntil(QFile& file, const QString &splitter);
    qint64 seekFirstRecord(QFile& file);
    static int seekTimeString(const QByteArray& str, const QString& format);
    static int getBomLength(QByteArray& str);
};

#endif // FILECONTAINER_H
