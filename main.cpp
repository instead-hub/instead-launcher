#include <QtGui/QApplication>
#include <QTextCodec>
#include "mainwindow.h"
#include <stdio.h>
#include <string.h>

FILE *logFile = 0;

void logMessageOutput(QtMsgType type, const char *msg)
 {
     switch (type) {
     case QtDebugMsg:
         fprintf(logFile, "Debug: %s\n", msg);
         break;
     case QtWarningMsg:
         fprintf(logFile, "Warning: %s\n", msg);
         break;
     case QtCriticalMsg:
         fprintf(logFile, "Critical: %s\n", msg);
         break;
     case QtFatalMsg:
         fprintf(logFile, "Fatal: %s\n", msg);
         abort();
     }
 }

void showHelp() {
    printf("instead-launcher [-log <logfile>] [-help]\n");
    printf("  -help   show this help message\n");
    printf("  -log    redirect debug messages to <logfile>\n\n");
}

bool enableLog(char *fileName) {
    logFile = fopen(fileName, "at");
    if (!logFile) {
        printf("Can't open file %s for writing\n", fileName);
        return false;
    }
    fprintf(logFile, "Log started\n");
    qInstallMsgHandler(logMessageOutput);
    return true;
}

int qtMain( int argc, char *argv[] ) {
    QApplication a(argc, argv);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));
    MainWindow w;
    w.show();
    return a.exec();
}

int main( int argc, char *argv[] )
{
    for (int i=1; i<argc; i++) {
        if ( !strcmp(argv[i], "-help") || !strcmp(argv[i], "--help") ) {
            showHelp();
            return 1;
        } else if ( !strcmp(argv[i], "-log") || !strcmp(argv[i], "--log") ) {
            i++;
            if (i < argc) {
                if (!enableLog(argv[i])) return 1;
            } else {
                showHelp();
                return 1;
            }
        } else {
            printf("Unrecognized option: %s\n\n", argv[i]);
            showHelp();
            return 1;
        }
    }

    int result = qtMain( argc, argv );

    if ( logFile ) {
        fprintf( logFile, "Log finished\n\n");
        fclose( logFile );
    }
    return result;
}
