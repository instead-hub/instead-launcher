#include "platform.h"
#include <QMessageBox>

bool checkOrCreateGameDir( QString gameDir ) {

    QDir dir(gameDir);
    if (dir.exists()) return true;

    qWarning() << "mkpath " << gameDir;

    return dir.mkpath(".");
}

#ifdef Q_OS_UNIX

QString getGameDirPath() {
    return QDir::home().absolutePath() + "/.instead/games/";
}

QString getConfigPath() {
    return QDir::home().absolutePath() + "/.instead/launcher.ini";
}

QString getDefaultInterpreterPath() {
    return "/usr/local/bin/sdl-instead";
}

#elif defined(Q_OS_WIN)

QString getGameDirPath() {
    QString currentPath = QDir::currentPath()+"/appdata";
    if (QFileInfo(currentPath).exists()) {
        return QDir::toNativeSeparators("appdata/games/");
    }
    return QDir::toNativeSeparators(QDir::home().absolutePath()) + "\\Local Settings\\Application Data\\instead\\games\\";
}

QString getConfigPath() {
    QString currentPath = QDir::currentPath()+"/appdata";
    if (QFileInfo(currentPath).exists()) {
        return QDir::toNativeSeparators(currentPath + "/launcher.ini");
    }
    return QDir::toNativeSeparators(QDir::home().absolutePath()) + "\\Local Settings\\Application Data\\instead\\launcher.ini";
}

QString getDefaultInterpreterPath() {
    QString currentPath = QDir::currentPath()+"/sdl-instead.exe";
    if (QFileInfo(currentPath).exists()) {
        return QDir::toNativeSeparators("sdl-instead.exe");
    }
    return "c:\\Program Files\\Games\\INSTEAD\\sdl-instead.exe";
}

#else

#error "Unsupported OS"

#endif
