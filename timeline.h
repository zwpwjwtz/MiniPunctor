#ifndef TIMELINE_H
#define TIMELINE_H

#include <QString>
#include <QList>

#define PUNCTOR_TIME_FIELD_START 1
#define PUNCTOR_TIME_FIELD_END 2
#define PUNCTOR_TIME_FIELD_ID 3
#define PUNCTOR_TIME_FIELD_CONTENT 4

#define PUNCTOR_TIME_FORMAT_MSEC "zzz"
#define PUNCTOR_TIME_FORMAT_CSEC "zz"
#define PUNCTOR_TIME_FORMAT_MSECS "Z"
#define PUNCTOR_TIME_FORMAT_SEC "ss"
#define PUNCTOR_TIME_FORMAT_SECS "S"
#define PUNCTOR_TIME_FORMAT_MIN "mm"
#define PUNCTOR_TIME_FORMAT_MINS "M"
#define PUNCTOR_TIME_FORMAT_HOUR "hh"
#define PUNCTOR_TIME_FORMAT_HOURS "H"
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
    const TimeTick &operator [](int index);
    const TimeTick& getItemByIndex(int index);
    void addItem(TimeTick& tick, int index);
    int addItemByTime(qint64 timeStamp);
    void appendItem(TimeTick& tick);
    void removeItem(int index);
    void updateItem(TimeTick& tick, int index);
    void swapItem(int index1, int index2);
    void sort(int field);
    void clear();
    bool existId(QString id);
    QString getUnusedId();
    void getIndexesByTime(qint64 timeStamp,
                          qint64 timeInterval,
                          QList<int>& indexList);
    static QString timeStampToString(qint64 timeStamp, QString format);
    static qint64 stringToTimeStamp(QString str, QString format);

private:
    QList<TimeTick> list;
};

#endif // TIMELINE_H
