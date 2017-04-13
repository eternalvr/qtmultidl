#-------------------------------------------------
#
# Project created by QtCreator 2017-03-23T11:59:07
#
#-------------------------------------------------

QT       += core gui network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MultiDownload
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp\
        dlwindow.cpp \
    logindialog.cpp \
    netmanager.cpp \
    configuration.cpp \
    mp3.cpp \
    downloadthread.cpp \
    deleteworker.cpp

HEADERS  += dlwindow.h \
    logindialog.h \
    netmanager.h \
    configuration.h \
    mp3.h \
    downloadthread.h \
    deleteworker.h

FORMS    += dlwindow.ui \
    logindialog.ui


win{
    LIBS += -LC:/OpenSSL-Win32/lib -lubsec
    INCLUDEPATH += C:/OpenSSL-Win32/include
}
win32:RC_ICONS += images/cloud.ico

DISTFILES += \
    fileview.qml

RESOURCES += \
    icons.qrc
