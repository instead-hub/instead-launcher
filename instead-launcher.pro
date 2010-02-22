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
    updatewidget.cpp

HEADERS += mainwindow.h \
    unzip/unzip.h \
    unzip/ioapi.h \
    qunzip.h \
    platform.h \
    updatewidget.h \
    config.h

FORMS += mainwindow.ui
#LIBS += -qt-zlib # use zlib embedded into qt
DEFINES += NOUNCRIPT # need for unzip library build
RESOURCES += instead-launcher.qrc
TRANSLATIONS += instead-launcher_ru.ts
win32:INCLUDEPATH += ${QTDIR}/src/3rdparty/zlib
OTHER_FILES += README.TXT
RC_FILE = resources.rc

unix:DESTDIR = .

unix:system(cat instead-launcher.desktop.in | sed -e "s\|@BIN\|$$[QT_INSTALL_BINS]\|g" > instead-launcher.desktop)
unix:system(find . -name *.ts | xargs $$LRELEASE_EXECUTABLE)
unix{
target.path = $$PREFIX/bin
desktop.files = instead-launcher.desktop
desktop.path = $$PREFIX/share/applications
INSTALLS += desktop target
}
