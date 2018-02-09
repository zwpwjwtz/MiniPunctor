#include "mediacontrol.h"

#include <QDBusInterface>
#include <QDBusReply>

#define PUNCTOR_DBUS_SERVICE_NAME "org.freedesktop.DBus"
#define PUNCTOR_DBUS_SERVICE_PATH "/"
#define PUNCTOR_DBUS_INTERFACE "org.freedesktop.DBus"
#define PUNCTOR_DBUS_METHOD_LISTNAMES "ListNames"

#define PUNCTOR_MPRIS_SERVICE_NAME "org.mpris.MediaPlayer2"
#define PUNCTOR_MPRIS_OBJECT_PATH "/org/mpris/MediaPlayer2"
#define PUNCTOR_MPRIS_INTERFACE "org.mpris.MediaPlayer2"
#define PUNCTOR_MPRIS_PROP_IDENTITY "Identity"

#define PUNCTOR_MPRIS_PLAYER_INTERFACE "org.mpris.MediaPlayer2.Player"
#define PUNCTOR_MPRIS_PLAYER_PROP_STATUS "PlaybackStatus"
#define PUNCTOR_MPRIS_PLAYER_PROP_POS "Position"
#define PUNCTOR_MPRIS_PLAYER_METHOD_PLAY "Play"
#define PUNCTOR_MPRIS_PLAYER_METHOD_PAUSE "Pause"
#define PUNCTOR_MPRIS_PLAYER_METHOD_RESUME "PlayPause"
#define PUNCTOR_MPRIS_PLAYER_METHOD_STOP "Stop"
#define PUNCTOR_MPRIS_PLAYER_METHOD_SEEK "Seek"
#define PUNCTOR_MPRIS_PLAYER_METHOD_PREV "Previous"
#define PUNCTOR_MPRIS_PLAYER_METHOD_NEXT "Next"

#define PUNCTOR_MPRIS_PLAYER_STATUS_STOPPED "Stopped"
#define PUNCTOR_MPRIS_PLAYER_STATUS_PLAYING "Playing"
#define PUNCTOR_MPRIS_PLAYER_STATUS_PAUSED "Paused"


MediaControl::MediaControl()
{
}

int MediaControl::getPlayerList(QList<QString>& playerName,
                            QList<QString>& playerID)
{
    int i;
    playerName.clear();
    playerID.clear();

    QDBusInterface iface(PUNCTOR_DBUS_SERVICE_NAME,
                         PUNCTOR_DBUS_SERVICE_PATH,
                         PUNCTOR_DBUS_INTERFACE);
    QDBusReply<QStringList> reply =
                                    iface.call(PUNCTOR_DBUS_METHOD_LISTNAMES);
    for (i=0; i<reply.value().count(); i++)
    {
        if (reply.value()[i].indexOf(PUNCTOR_MPRIS_SERVICE_NAME) >= 0)
            playerID.append(reply.value()[i]);
    }

    QDBusInterface* iface2;
    for (i=0; i<playerID.count(); i++)
    {
        iface2 = new QDBusInterface(playerID[i],
                                    PUNCTOR_MPRIS_OBJECT_PATH,
                                    PUNCTOR_MPRIS_INTERFACE);
        playerName.append(iface2->property(PUNCTOR_MPRIS_PROP_IDENTITY)
                                .toString());
        delete iface2;
    }

    return playerName.count();
}

MediaControl::MediaStatus MediaControl::getStatus(QString playerID)
{
    QDBusInterface iface(playerID,
                         PUNCTOR_MPRIS_OBJECT_PATH,
                         PUNCTOR_MPRIS_PLAYER_INTERFACE);
    QString status = iface.property(PUNCTOR_MPRIS_PLAYER_PROP_STATUS)
                          .toString();
    if (status == PUNCTOR_MPRIS_PLAYER_STATUS_STOPPED)
        return status_stopped;
    else if (status == PUNCTOR_MPRIS_PLAYER_STATUS_PLAYING)
        return status_playing;
    else if (status == PUNCTOR_MPRIS_PLAYER_STATUS_PAUSED)
        return status_paused;
    return status_unknown;
}

void MediaControl::play(QString playerID)
{
    QDBusInterface iface(playerID,
                         PUNCTOR_MPRIS_OBJECT_PATH,
                         PUNCTOR_MPRIS_PLAYER_INTERFACE);
    QString status = iface.property(PUNCTOR_MPRIS_PLAYER_PROP_STATUS)
                          .toString();
    if (status == PUNCTOR_MPRIS_PLAYER_STATUS_STOPPED)
        iface.call(PUNCTOR_MPRIS_PLAYER_METHOD_PLAY);
    else if (status == PUNCTOR_MPRIS_PLAYER_STATUS_PAUSED)
        iface.call(PUNCTOR_MPRIS_PLAYER_METHOD_RESUME);
}

void MediaControl::pause(QString playerID)
{
    QDBusInterface iface(playerID,
                         PUNCTOR_MPRIS_OBJECT_PATH,
                         PUNCTOR_MPRIS_PLAYER_INTERFACE);
    QString status = iface.property(PUNCTOR_MPRIS_PLAYER_PROP_STATUS)
                          .toString();
    if (status == PUNCTOR_MPRIS_PLAYER_STATUS_PLAYING)
        iface.call(PUNCTOR_MPRIS_PLAYER_METHOD_PAUSE);
}

void MediaControl::stop(QString playerID)
{
    QDBusInterface iface(playerID,
                         PUNCTOR_MPRIS_OBJECT_PATH,
                         PUNCTOR_MPRIS_PLAYER_INTERFACE);
    QString status = iface.property(PUNCTOR_MPRIS_PLAYER_PROP_STATUS)
                          .toString();
    if (status == PUNCTOR_MPRIS_PLAYER_STATUS_PLAYING)
        iface.call(PUNCTOR_MPRIS_PLAYER_METHOD_STOP);
}

void MediaControl::seek(QString playerID, qint64 offset)
{
    QDBusInterface iface(playerID,
                         PUNCTOR_MPRIS_OBJECT_PATH,
                         PUNCTOR_MPRIS_PLAYER_INTERFACE);
    iface.call(PUNCTOR_MPRIS_PLAYER_METHOD_SEEK, offset * 1000);
}

qint64 MediaControl::getPosition(QString playerID)
{
    QDBusInterface iface(playerID,
                         PUNCTOR_MPRIS_OBJECT_PATH,
                         PUNCTOR_MPRIS_PLAYER_INTERFACE);
    return iface.property(PUNCTOR_MPRIS_PLAYER_PROP_POS).toLongLong() / 1000;
}

void MediaControl::previous(QString playerID)
{
    QDBusInterface iface(playerID,
                         PUNCTOR_MPRIS_OBJECT_PATH,
                         PUNCTOR_MPRIS_PLAYER_INTERFACE);
    iface.call(PUNCTOR_MPRIS_PLAYER_METHOD_PREV);
}

void MediaControl::next(QString playerID)
{
    QDBusInterface iface(playerID,
                         PUNCTOR_MPRIS_OBJECT_PATH,
                         PUNCTOR_MPRIS_PLAYER_INTERFACE);
    iface.call(PUNCTOR_MPRIS_PLAYER_METHOD_NEXT);
}
