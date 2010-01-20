#ifndef PLATFORM_H
#define PLATFORM_H
#include <QtCore>

// Platform specific functions.

bool checkOrCreateGameDir( QString gameDir );
QString getGameDirPath();
QString getConfigPath();
QString getDefaultInterpreterPath();

#endif // PLATFORM_H
