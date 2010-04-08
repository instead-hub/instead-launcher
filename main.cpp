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
    printf("instead-launcher [-insteadpath <dir>] [-gamespath <dir>] [-log <logfile>] [-help]\n");
    printf("  -insteadpath   set instead dir\n");
    printf("  -gamespath     set games path dir\n");
    printf("  -default-insteadpath   set default instead dir\n");
    printf("  -default-gamespath     set default games path dir\n");
    printf("  -help          show this help message\n");
    printf("  -log           redirect debug messages to <logfile>\n\n");
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

int qtMain( const ArgMap &argMap, int argc, char *argv[] ) {
    QApplication a(argc, argv);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));
    MainWindow w( argMap );
    w.show();
    return a.exec();
}

int main( int argc, char *argv[] )
{
    ArgMap argMap;

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
        } else if ( !strcmp(argv[i], "-gamespath") || !strcmp(argv[i], "--gamespath") ) {
            i++;
            if (i < argc) {
        	argMap["gamespath"]=QString::fromLocal8Bit( argv[i] );
            } else {
                showHelp();
                return 1;
            }
        } else if ( !strcmp(argv[i], "-insteadpath") || !strcmp(argv[i], "--insteadpath") ) {
            i++;
            if (i < argc) {
        	if ( QFileInfo( QString::fromLocal8Bit( argv[i] ) ).exists() ) {
        	    argMap["insteadpath"]=QString::fromLocal8Bit( argv[i] );
        	} else {
        	    showHelp();
        	    return 1;
        	}
            } else {
                showHelp();
                return 1;
            }
        } else if ( !strcmp(argv[i], "-default-gamespath") || !strcmp(argv[i], "--default-gamespath") ) {
            i++;
            if (i < argc) {
        	argMap["default-gamespath"]=QString::fromLocal8Bit( argv[i] );
            } else {
                showHelp();
                return 1;
            }
        } else if ( !strcmp(argv[i], "-default-insteadpath") || !strcmp(argv[i], "--default-insteadpath") ) {
            i++;
            if (i < argc) {
        	if ( QFileInfo( QString::fromLocal8Bit( argv[i] ) ).exists() ) {
        	    argMap["default-insteadpath"]=QString::fromLocal8Bit( argv[i] );
        	} else {
        	    showHelp();
        	    return 1;
        	}
            } else {
                showHelp();
                return 1;
            }
        } else {
            fprintf(stderr,"Unrecognized option: %s\n\n", argv[i]);
            showHelp();
            return 1;
        }
    }

    int result = qtMain( argMap, argc, argv );

    if ( logFile ) {
        fprintf( logFile, "Log finished\n\n");
        fclose( logFile );
    }
    return result;
}
