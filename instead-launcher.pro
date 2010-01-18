# -------------------------------------------------
# Project created by QtCreator 2010-01-17T11:48:50
# -------------------------------------------------
QT += network \
    xml
TARGET = instead-launcher
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    unzip/unzip.c \
    unzip/ioapi.c \
    qunzip.cpp
HEADERS += mainwindow.h \
    unzip/unzip.h \
    unzip/ioapi.h \
    qunzip.h
FORMS += mainwindow.ui
LIBS += -qt-zlib # use zlib embedded into qt
DEFINES += NOUNCRIPT # need for unzip library build
RESOURCES += instead-launcher.qrc

win32 {
    INCLUDEPATH += ${QTDIR}/src/3rdparty/zlib
    DEFINES += Q_OS_WIN32
}
