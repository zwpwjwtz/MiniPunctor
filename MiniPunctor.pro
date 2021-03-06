#-------------------------------------------------
#
# Project created by QtCreator 2017-01-26T18:44:04
#
#-------------------------------------------------

QT       += core gui dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MiniPunctor
TEMPLATE = app

VER_MAJ = 1
VER_MIN = 5
VER_PAT = 0
VERSION = 1.5.0
VERSION_PE_HEADER = 1.5.0

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS APP_VERSION=\\\"$$VERSION\\\"

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    helpwindow.cpp \
    buttonupdown.cpp \
    filecontainer.cpp \
    timeline.cpp \
    tickeditwindow.cpp \
    searchwindow.cpp \
    tickshiftwindow.cpp \
    mediacontrol.cpp

HEADERS  += mainwindow.h \
    helpwindow.h \
    buttonupdown.h \
    filecontainer.h \
    timeline.h \
    file_format.h \
    tickeditwindow.h \
    searchwindow.h \
    tickshiftwindow.h \
    mediacontrol.h

FORMS    += mainwindow.ui \
    helpwindow.ui \
    buttonupdown.ui \
    tickeditwindow.ui \
    searchwindow.ui \
    tickshiftwindow.ui

RESOURCES += \
    icon.qrc

target.path = $${PREFIX}/bin/

INSTALLS += target
