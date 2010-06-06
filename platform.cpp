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
    QString path = QDir::toNativeSeparators(QDir::home().absolutePath()) + "\\Local Settings\\Application Data\\instead\\games\\";
    QString currentPath = QDir::currentPath()+"\\appdata";
    if (QFileInfo(currentPath).exists()) {
	path=currentPath;
    }
    return path;
}

QString getConfigPath() {
    return QDir::toNativeSeparators(QDir::home().absolutePath()) + "\\Local Settings\\Application Data\\instead\\launcher.ini";
}

QString getDefaultInterpreterPath() {
    return "c:\\Program Files\\Pinebrush games\\INSTEAD\\sdl-instead.exe";
}

#else

#error "Unsupported OS"

#endif
