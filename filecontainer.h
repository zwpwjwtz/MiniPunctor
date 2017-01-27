#ifndef FILECONTAINER_H
#define FILECONTAINER_H

#include <QString>
#include <QList>

class FileContainer
{
public:
    FileContainer();
    void save(QList<QString> &recordList);
    void saveAs(QList<QString> &recordList, QString path);

    QString filePath;
};

#endif // FILECONTAINER_H
