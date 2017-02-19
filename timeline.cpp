#include "timeline.h"


const char* Punctor_getTimeFieldFormat(int value, int minLength);

TimeLine::TimeLine()
{
}

int TimeLine::count()
{
    return list.count();
}

TimeTick &TimeLine::operator [](int index)
{
    return list[index];
}

TimeTick& TimeLine::getItemByIndex(int index)
{
    return list[index];
}

void TimeLine::addItem(TimeTick& tick, int index)
{
    if (tick.id.isEmpty())
        tick.id = getUnusedId();
    list.insert(index, tick);
}

int TimeLine::addItemByTime(qint64 timeStamp)
{
    TimeTick tick;
    tick.startTime = tick.endTime = timeStamp;
    int count = list.count();
    int i;
    for(i=0; i<count; i++)
    {
        if (list[i].startTime > timeStamp) break;
    }
    addItem(tick, i);
    return i;
}

void TimeLine::appendItem(TimeTick& tick)
{
    list.append(tick);
}

void TimeLine::removeItem(int index)
{
    list.removeAt(index);
}

void TimeLine::updateItem(TimeTick& tick, int index)
{
    list.replace(index, tick);
}
void TimeLine::swapItem(int index1, int index2)
{
    list.swap(index1, index2);
}

bool TimeLine::existId(QString id)
{
    bool found = false;
    QList<TimeTick>::const_iterator i;
    for(i=list.constBegin(); i!=list.constEnd(); i++)
    {
        if (i->id == id)
        {
            found = true;
            break;
        }
    }
    return found;
}

void TimeLine::clear()
{
    list.clear();
}

QString TimeLine::getUnusedId()
{
    uint tempNum = list.count() + 1;
    bool found = false;

    QList<TimeTick>::const_iterator i;
    for(i=list.constBegin(); i!=list.constEnd(); i++)
    {
        if (i->id == QString::number(tempNum))
            tempNum++;
        else
        {
            found = true;
            break;
        }
        if (tempNum == 0xFFFFFFFF)
            break;
    }
    if (found || tempNum == 1)
        return QString::number(tempNum);
    else
        return "";
}

const char* Punctor_getTimeFieldFormat(int value, int minLength)
{
    if (value < 10 && minLength < 2)
        return "%d";
    else if (value < 100 && minLength < 3)
        return "%02d";
    else if (value < 1000 && minLength < 4)
        return "%03d";
    else if (value < 10000 && minLength < 5)
        return "%04d";
    else if (value < 100000 && minLength < 6)
        return "%05d";
    else if (value < 1000000 && minLength < 7)
        return "%06d";
    else if (value < 10000000 && minLength < 8)
        return "%07d";
    else if (value < 100000000 && minLength < 9)
        return "%08d";
    else if (value < 1000000000 && minLength < 10)
        return "%09d";
    else
        return "";
}

QString TimeLine::timeStampToString(qint64 timeStamp, QString format)
{
    QString tempTime(format);
    int tempInt;
    static char buf[8];

    timeStamp = ::abs(timeStamp) % 360000000;
    if (format.contains(PUNCTOR_TIME_FORMAT_HOUR))
    {
        tempInt = timeStamp / 3600000;
        std::sprintf(buf, Punctor_getTimeFieldFormat(tempInt, 2), tempInt);
        tempTime.replace(PUNCTOR_TIME_FORMAT_HOUR, buf);
        timeStamp %= 3600000;
    }
    if (format.contains(PUNCTOR_TIME_FORMAT_MIN))
    {
        tempInt = timeStamp / 60000;
        std::sprintf(buf, Punctor_getTimeFieldFormat(tempInt, 2), tempInt);
        tempTime.replace(PUNCTOR_TIME_FORMAT_MIN, buf);
        timeStamp %= 60000;
    }
    if (format.contains(PUNCTOR_TIME_FORMAT_SEC))
    {
        tempInt = timeStamp / 1000;
        std::sprintf(buf, Punctor_getTimeFieldFormat(tempInt, 2), tempInt);
        tempTime.replace(PUNCTOR_TIME_FORMAT_SEC, buf);
        timeStamp %= 1000;
    }
    if (format.contains(PUNCTOR_TIME_FORMAT_MSEC))
    {
        std::sprintf(buf, Punctor_getTimeFieldFormat(timeStamp, 3), timeStamp);
        tempTime.replace(PUNCTOR_TIME_FORMAT_MSEC, buf);
    }
    else if (format.contains(PUNCTOR_TIME_FORMAT_CSEC))
    {
        tempInt = timeStamp / 10;
        std::sprintf(buf, Punctor_getTimeFieldFormat(tempInt, 2), tempInt);
        tempTime.replace(PUNCTOR_TIME_FORMAT_CSEC, buf);
    }

    return tempTime;
}

qint64 TimeLine::stringToTimeStamp(QString str, QString format)
{
    if (format.isEmpty())
        format = PUNCTOR_TIME_FORMAT_DEFAULT;
    qint64 tempTime = 0;
    int p, fraction;
    bool OK = true, valid = false;

    p = format.indexOf(PUNCTOR_TIME_FORMAT_MSEC);
    if (p >= 0)
    {
        fraction = str.mid(p, ::strlen(PUNCTOR_TIME_FORMAT_MSEC)).toInt(&OK);
        tempTime += fraction;
        valid = true;
    }
    else
    {
        p = format.indexOf(PUNCTOR_TIME_FORMAT_CSEC);
        if (p >= 0 && OK)
        {
            fraction = str.mid(p, ::strlen(PUNCTOR_TIME_FORMAT_CSEC))
                          .toInt(&OK);
            tempTime += fraction * 10;
            valid = true;
        }
    }

    p = format.indexOf(PUNCTOR_TIME_FORMAT_SEC);
    if (p >= 0 && OK)
    {
        fraction = str.mid(p, ::strlen(PUNCTOR_TIME_FORMAT_SEC)).toInt(&OK);
        tempTime += fraction * 1000;
        valid = true;
    }

    p = format.indexOf(PUNCTOR_TIME_FORMAT_MIN);
    if (p >= 0 && OK)
    {
        fraction = str.mid(p, ::strlen(PUNCTOR_TIME_FORMAT_MIN)).toInt(&OK);
        tempTime += fraction * 60000;
        valid = true;
    }

    p = format.indexOf(PUNCTOR_TIME_FORMAT_HOUR);
    if (p >= 0 && OK)
    {
        fraction = str.mid(p, ::strlen(PUNCTOR_TIME_FORMAT_HOUR)).toInt(&OK);
        tempTime += fraction * 3600000;
        valid = true;
    }

    if (OK && valid)
        return tempTime;
    else
        return -1;
}
