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
#define PUNCTOR_MPRIS_PLAYER_METHOD_STOP "Stop"
#define PUNCTOR_MPRIS_PLAYER_METHOD_SEEK "Seek"


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
    if (status == "Stopped")
        return status_stopped;
    return status_unknown;
}

void MediaControl::play(QString playerID)
{
    QDBusInterface iface(playerID,
                         PUNCTOR_MPRIS_OBJECT_PATH,
                         PUNCTOR_MPRIS_PLAYER_INTERFACE);
    iface.call(PUNCTOR_MPRIS_PLAYER_METHOD_PLAY);
}

void MediaControl::pause(QString playerID)
{
    QDBusInterface iface(playerID,
                         PUNCTOR_MPRIS_OBJECT_PATH,
                         PUNCTOR_MPRIS_PLAYER_INTERFACE);
    iface.call(PUNCTOR_MPRIS_PLAYER_METHOD_PAUSE);
}

void MediaControl::stop(QString playerID)
{
    QDBusInterface iface(playerID,
                         PUNCTOR_MPRIS_OBJECT_PATH,
                         PUNCTOR_MPRIS_PLAYER_INTERFACE);
    iface.call(PUNCTOR_MPRIS_PLAYER_METHOD_STOP);
}

void MediaControl::seek(QString playerID, qint64 pos)
{
    QDBusInterface iface(playerID,
                         PUNCTOR_MPRIS_OBJECT_PATH,
                         PUNCTOR_MPRIS_PLAYER_INTERFACE);
    iface.call(PUNCTOR_MPRIS_PLAYER_METHOD_SEEK, pos);
}

qint64 MediaControl::getPosition(QString playerID)
{
    QDBusInterface iface(playerID,
                         PUNCTOR_MPRIS_OBJECT_PATH,
                         PUNCTOR_MPRIS_PLAYER_INTERFACE);
    return iface.property(PUNCTOR_MPRIS_PLAYER_PROP_POS).toLongLong();
}
