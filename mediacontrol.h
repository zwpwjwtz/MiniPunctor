#ifndef MEDIACONTROL_H
#define MEDIACONTROL_H

#include <QObject>
#include <QList>

class MediaControl : public QObject
{
    Q_OBJECT
public:
    enum MediaStatus
    {
        status_ready = 0,
        status_playing = 1,
        status_paused = 2,
        status_stopped = 3,
        status_unknown = 255,
    };

    MediaControl();

    int getPlayerList(QList<QString>& playerName, QList<QString>& playerID);
    MediaStatus getStatus(QString playerID);
    void play(QString playerID);
    void pause(QString playerID);
    void stop(QString playerID);
    void seek(QString playerID, qint64 pos);
    qint64 getPosition(QString playerID);
    void previous(QString playerID);
    void next(QString playerID);
};

#endif // MEDIACONTROL_H
