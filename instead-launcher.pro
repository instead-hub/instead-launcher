# -------------------------------------------------
# Project created by QtCreator 2010-01-17T11:48:50
# -------------------------------------------------
QT += network
TARGET = instead-launcher
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    unzip/unzip.c \
    unzip/ioapi.c \
    qunzip.cpp \
    platform.cpp \
    updatewindow.cpp
HEADERS += mainwindow.h \
    unzip/unzip.h \
    unzip/ioapi.h \
    qunzip.h \
    platform.h \
    updatewindow.h \
    config.h
FORMS += mainwindow.ui \
    updatewindow.ui
LIBS += -qt-zlib # use zlib embedded into qt
DEFINES += NOUNCRIPT # need for unzip library build
RESOURCES += instead-launcher.qrc
TRANSLATIONS += instead-launcher_ru.ts
win32:INCLUDEPATH += ${QTDIR}/src/3rdparty/zlib
OTHER_FILES += README.TXT
