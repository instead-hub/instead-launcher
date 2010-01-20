#include "platform.h"
#include <QMessageBox>

bool checkOrCreateGameDir( QString gameDir ) {

    QDir dir(gameDir);
    if (dir.exists()) return true;

    qWarning() << "mkpath " << gameDir;

    return dir.mkpath( gameDir );
}

QString getGameDirPath()
{
#ifdef Q_OS_UNIX
    return QDir::home().absolutePath() + "/.instead/games/";

#elif defined(Q_OS_WIN)
    return QDir::toNativeSeparators(QDir::home().absolutePath()) + "\\Local Settings\\Application Data\\instead\\games\\";

#else
#error "Unsupported OS"
#endif
}

QString getConfigPath()
{
#ifdef Q_OS_UNIX
    return QDir::home().absolutePath() + "/.instead/launcher.ini";

#elif defined(Q_OS_WIN)
    return QDir::toNativeSeparators(QDir::home().absolutePath()) + "\\Local Settings\\Application Data\\instead\\launcher.ini";

#else
#error "Unsupported OS"
#endif
}

QString getDefaultInterpreterPath()
{
#ifdef Q_OS_UNIX
    return "/usr/local/bin/sdl-instead";

#elif defined(Q_OS_WIN)
    return "c:\\Program Files\\Pinebrush games\\INSTEAD\\sdl-instead.exe";

#else
#error "Unsupported OS"
#endif
}
