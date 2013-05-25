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
    updatewidget.cpp \
    global.cpp \
    aboutwidget.cpp \
    urlresolver.cpp

HEADERS += mainwindow.h \
    unzip/unzip.h \
    unzip/ioapi.h \
    qunzip.h \
    platform.h \
    updatewidget.h \
    config.h \
    global.h \
    aboutwidget.h \
    urlresolver.h

FORMS += mainwindow.ui
#LIBS += -qt-zlib # use zlib embedded into qt
LIBS += -lz
DEFINES += NOUNCRIPT # need for unzip library build
RESOURCES += instead-launcher.qrc
TRANSLATIONS += instead-launcher_ru.ts
win32:INCLUDEPATH += ${QTDIR}/src/3rdparty/zlib
OTHER_FILES += README.TXT
RC_FILE = resources.rc

unix:exists($$[QT_INSTALL_BINS]/lrelease){
LRELEASE_EXECUTABLE = $$[QT_INSTALL_BINS]/lrelease
}

unix:exists($$[QT_INSTALL_BINS]/lrelease-qt4){
LRELEASE_EXECUTABLE = $$[QT_INSTALL_BINS]/lrelease-qt4
}

win32:exists($$[QT_INSTALL_BINS]/lrelease.exe){
LRELEASE_EXECUTABLE = $$[QT_INSTALL_BINS]/lrelease.exe
}

isEmpty(LRELEASE_EXECUTABLE){
error(Could not find lrelease executable)
}
else {
message(Found lrelease executable: $$LRELEASE_EXECUTABLE)
}

message(generating translations)
unix:system(find . -name \*.ts | xargs $$LRELEASE_EXECUTABLE)
win32:system(for /r %B in (*.ts) do $$LRELEASE_EXECUTABLE %B)

unix:DESTDIR = .

unix:system(cat instead-launcher.desktop.in | sed -e "s\|@BIN\|$$PREFIX\/bin\|g" > instead-launcher.desktop)
unix{
isEmpty(PREFIX){
	PREFIX=/usr/local
}
target.path = $$PREFIX/bin
desktop.files = instead-launcher.desktop
desktop.path = $$PREFIX/share/applications
INSTALLS += desktop target
}
