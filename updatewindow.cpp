#include "updatewindow.h"
#include "ui_updatewindow.h"
#include <QtGui>
#include <QtCore>
#include <QtNetwork>
#include "config.h"

UpdateWindow::UpdateWindow(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::UpdateWindow)
{
    m_ui->setupUi(this);

    m_listServer = new QHttp(this);
    connect( m_listServer, SIGNAL( done( bool ) ), this, SLOT( listServerDone( bool ) ) );

    m_listLoadProgress = new QProgressDialog(parent);
    m_listLoadProgress->setLabelText( tr( "Update list downloading" ) + "..." );
    connect( m_listLoadProgress, SIGNAL(canceled()), m_listServer, SLOT(abort()));
    connect( m_listServer, SIGNAL( dataReadProgress( int, int ) ), m_listLoadProgress, SLOT( setValue( int ) ) );

    localInsteadVersion = "";
    localLauncherVersion = "";
    remoteInsteadVersion = "";
    remoteLauncherVersion = "";
    urlInstead = "";
    urlLauncher = "";

}

UpdateWindow::~UpdateWindow()
{
    qDebug("~UpdateWindow");
    delete m_ui;
}

void UpdateWindow::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void UpdateWindow::refreshUpdateList( QString insteadBinary, bool automatically ) {
    // local versions of launcher
    localLauncherVersion = LAUNCHER_VERSION;
    localInsteadVersion = detectInsteadVersion(insteadBinary);
    // retrieve remote versions
    m_automatically = automatically;
    m_ui->textBrowser->setHtml( "<h3>" + tr("Loading updates ... please wait") + "</h3>" );
    QUrl url(SW_UPDATE_URL);
    qDebug() << "Downloading update list from " << url.toString();
    m_listServer->setHost(url.host());
    setEnabled( false );
    m_listServer->get(url.path());
    m_listLoadProgress->setValue(0);
}

void UpdateWindow::listServerDone( bool error ) {
    qDebug( "Update list has been downloaded" );

    setEnabled( true );
    m_listLoadProgress->reset();
    if(!error) {
        QXmlStreamReader xml( m_listServer->readAll() );
        while (!xml.atEnd()) {
            xml.readNext();
            if (xml.isStartElement() && xml.name() == "updates") {
                parseUpdateList(&xml);
                break; // it should be only one update list
            }
        }
        if (xml.hasError()) {
            qWarning("Warning: errors while parsing update list");
            const QString s = xml.errorString();
            qWarning() << s;
        } else {
            // all right, detect instead version
            generateUpdateMessage();
        }
    }
    else {
        qWarning("WARN: errors while downloading");
        QMessageBox::critical(this, tr("Error"), tr("Can't download update list"));
    }
}

void UpdateWindow::parseUpdateList( QXmlStreamReader *xml ) {
    Q_ASSERT( xml->name() == "updates" );
    while( !xml->atEnd() ) {
        xml->readNext();
        if ( xml->isStartElement() && xml->name() == "instead" ) {
            while ( !xml->atEnd() ) {
                xml->readNext();
                if ( xml->isStartElement() ) {
                    if ( xml->name() == "version" ) {
                        remoteInsteadVersion = xml->readElementText();
                    } else if ( xml->name() == "url" ) {
                        urlInstead = xml->readElementText();
                    }
                } else if ( xml->isEndElement() && xml->name()=="instead" ) {
                    break;
                }
            }
        } else  if ( xml->isStartElement() && xml->name() == "launcher" ) {
            while ( !xml->atEnd() ) {
                xml->readNext();
                if ( xml->isStartElement() ) {
                    if ( xml->name() == "version" ) {
                        remoteLauncherVersion = xml->readElementText();
                    } else if ( xml->name() == "url" ) {
                        urlLauncher = xml->readElementText();
                    }
                } else if ( xml->isEndElement() && xml->name()=="launcher" ) {
                    break;
                }
            }
        }
    }
}

void UpdateWindow::generateUpdateMessage() {
    qDebug() << "Instead version " << remoteInsteadVersion << "at url" << urlInstead;
    qDebug() << "Launcher version " << remoteLauncherVersion << "at url" << urlLauncher;
    qDebug() << "Local instead version " << localInsteadVersion;
    qDebug() << "Local launcher version " << localLauncherVersion;

    bool needUpdateInstead = false;
    bool needUpdateLauncher = false;

    QString text;

    if (localInsteadVersion < remoteInsteadVersion) {
        if ( localInsteadVersion == "0" ) {
            text += "<h3>" + tr("Instead update to ") + remoteInsteadVersion + "</h3>";
        } else {
            text += "<h3>" + tr("Instead update from ") + localInsteadVersion + tr(" to ") + remoteInsteadVersion + "</h3>";
        }
        text += "<a href=\"" + urlInstead + "\">" + urlInstead + "</a>";
        needUpdateInstead = true;
    }
    if (localLauncherVersion < remoteLauncherVersion) {
        text += "<h3>" + tr("Launcher update from ") + localLauncherVersion + tr(" to ") + remoteLauncherVersion + "</h3>";
        text += "<a href=\"" + urlLauncher + "\">" + urlLauncher + "</a>";
        needUpdateLauncher = true;
    }

    if (!needUpdateInstead && !needUpdateLauncher) {
        // auto close window if no updates available
        if ( m_automatically ) {
            accept();
            return;
        }
        text += "<h3>"+tr("No updates available")+"</h3>";
    }

    m_ui->textBrowser->setHtml(text);

    if (m_automatically) show();

}

void UpdateWindow::checkUpdates( QWidget *parent, QString insteadBinary, bool automatically ) {
    UpdateWindow *window = new UpdateWindow(parent);
    window->setAttribute(Qt::WA_DeleteOnClose, true);
    window->setWindowModality(Qt::WindowModal);
    if ( !automatically ) window->show();
    window->refreshUpdateList( insteadBinary, automatically );
}

QString UpdateWindow::detectInsteadVersion( QString insteadBinary ) {
    QString version = "0";
    QDir tempDir = QDir::temp();
    tempDir.remove("instead-version/main.lua");
    tempDir.remove("instead-version/version.txt");
    tempDir.rmdir("instead-version");
    if (tempDir.mkdir( "instead-version" )) {
        QFile file(tempDir.absolutePath() + "/instead-version/main.lua");
        if (file.open(QIODevice::WriteOnly)) {
            // generate main.lua
            QTextStream stream(&file);
            QString savePath = QDir::toNativeSeparators(tempDir.absolutePath() + "/instead-version/version.txt");
            stream << "-- $Name: Version$\n";
            stream << "f = io.open(\"" + QDir::fromNativeSeparators(savePath) + "\", \"w\");\n";
            stream << "if type(stead) ~= 'table' then\n";
            stream << "  io.output(f):write(version)\n";
            stream << "else\n";
            stream << "io.output(f):write(stead.version)\n";
            stream << "end\n";
            stream << "io.close(f);\n";
            stream << "os.exit(123);\n";
            file.close();
            // launch process
            QStringList arguments;
            arguments << "-game" << "instead-version";
            arguments << "-nostdgames";
            arguments << "-gamespath" << QDir::toNativeSeparators(tempDir.absolutePath());
            QFileInfo info(insteadBinary);
            QProcess process(this);
            process.setWorkingDirectory( info.path() );
            qDebug() << "Launch " << insteadBinary << " with args " << arguments;
            // execute doesn't work for windows instead version, sorry :(
            process.start( insteadBinary, arguments );
            process.waitForFinished(2000);
            qDebug() << "Instead finished with exit code " << process.exitCode();
            if (process.exitCode() != 123) {
                qWarning() << "Wrong exit code";
            } else {
                QFile verFile(savePath);
                if (verFile.open(QIODevice::ReadOnly)) {
                    QTextStream verStream(&verFile);
                    version = verStream.readLine();
                    verFile.close();
                } else {
                    qWarning() << "Can't open version.txt";
                }
            }
        } else {
            qWarning() << "Can't create main.lua";
        }
        tempDir.remove("instead-version/main.lua");
        tempDir.remove("instead-version/version.txt");
        tempDir.rmdir("instead-version");
    } else {
        qWarning() << "Can't create temp directory";
    }
    return version;
}

/* QString UpdateWindow::detectInsteadVersion( QString insteadBinary ) {

    qDebug() << "Instead binary at " << insteadBinary;

    QFile file( insteadBinary );
    if ( !file.open( QIODevice::ReadOnly ) ) {
        qWarning( "Can't open instead binary for reading " );
        return "0";
    }

    // stupid state machine

    char *prefix = "<[VERSION ";
    char *suffix = "]>";
    int state = 0;
    // 0 - wait for first letter of prefix
    // 1 - wait for letter of prefix
    // 2 - wait digit
    // 3 - wait digit or . or first letter of suffix
    // 4 - wait for letter of suffix
    int pos = 0; // position in prefix/suffix
    char buf;
    bool reuse = false; // reuse last character again
    QString ver = "";

    while (!file.atEnd()) {

        if ( reuse ) {
            reuse = false;
        } else {
            file.read( &buf, 1 );
        }

        switch ( state ) {
            case 0:
                if ( buf == prefix[0] ) {
                    state = 1;
                    pos = 1;
                }
                break;

            case 1:
                if ( !prefix[pos] ) {
                    state = 2;
                    ver = "";
                    reuse = true;
                } else if ( buf != prefix[pos]) {
                    state = 0;
                    reuse = true;
                } else {
                    ++pos;
                }
                break;

            case 2:
                if ( buf >= '0' && buf <= '9' ) {
                    ver += buf;
                    state = 3;
                } else {
                    state = 0;
                    reuse = true;
                }
                break;

            case 3:
                if ( buf == suffix[0] ) {
                    state = 4;
                    pos = 1;
                } else if ( buf == '.' ) {
                    ver += buf;
                    state = 2;
                } else if ( buf >= '0' && buf <= '9' ) {
                    ver += buf;
                } else {
                    state = 0;
                    reuse = true;
                }
                break;

            case 4:
                if ( !suffix[pos] ) {
                    qDebug() << "Instead version detected: " << ver;
                    return ver;
                } else if ( buf != suffix[pos] ) {
                    state = 0;
                    reuse = true;
                } else {
                    ++pos;
                }
                break;

        }

    }

    qWarning() << "No version string found in binary";
    return "0";
} */
