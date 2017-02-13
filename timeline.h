#ifndef TIMELINE_H
#define TIMELINE_H

#include <QList>

#define PUNCTOR_TIME_FORMAT_MSEC "zzz"
#define PUNCTOR_TIME_FORMAT_SEC "ss"
#define PUNCTOR_TIME_FORMAT_MIN "mm"
#define PUNCTOR_TIME_FORMAT_HOUR "hh"
#define PUNCTOR_TIME_FORMAT_DEFAULT "hh:mm:ss.zzz"


struct TimeTick
{
    qint64 startTime;
    qint64 endTime;
    QString id;
    QString content;
};


class TimeLine
{
public:
    TimeLine();
    int count();
    TimeTick& operator [](int index);
    TimeTick& getItemByIndex(int index);
    void addItem(TimeTick& tick, int index);
    int addItemByTime(qint64 timeStamp);
    void appendItem(TimeTick& tick);
    void removeItem(int index);
    void updateItem(TimeTick& tick, int index);
    void swapItem(int index1, int index2);
    void clear();
    bool existId(QString id);
    QString getUnusedId();
    static QString timeStampToString(qint64 timeStamp, QString format);
    static qint64 stringToTimeStamp(QString str, QString format);

private:
    QList<TimeTick> list;
};

#endif // TIMELINE_H
