#include "filecontainer.h"
#include <QFile>

FileContainer::FileContainer()
{
}

void FileContainer::save(QList<QString> &recordList)
{
}

void FileContainer::saveAs(QList<QString> &recordList, QString path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    foreach (QString record, recordList) {
        file.write(record.append("\n").toLocal8Bit());
    }
    file.close();
}
