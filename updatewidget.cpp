#include "updatewidget.h"
#include "config.h"

// -1  ver1 <  ver2
// 0   ver1 == ver2
// 1   ver1 >  ver2
int UpdateWidget::compareVersions( QString ver1, QString ver2 ) {
    QStringList v1 = ver1.split(".");
    QStringList v2 = ver2.split(".");
    int n1 = v1.count(), n2 = v2.count(), n = qMax( n1, n2 );
    for ( int i = 0; i < n; i++ ) {
        int d1 = 0, d2 = 0;
        if ( i < n1 ) {
            bool ok;
            d1 = v1[i].toInt( &ok );
            if ( !ok ) d1 = 0;
        }
        if ( i < n2 ) {
            bool ok;
            d2 = v2[i].toInt( &ok );
            if ( !ok ) d2 = 0;
        }
        if ( d1 > d2 ) return 1;
        if ( d1 < d2 ) return -1;
    }
    return 0;
}

UpdateWidget::UpdateWidget( QWidget *parent ) :
    QTextBrowser( parent )
{
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

    hide();

}

UpdateWidget::~UpdateWidget()
{
    qDebug("~UpdateWindow");
}

/*
void UpdateWidget::changeEvent(QEvent *e)
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
*/

void UpdateWidget::refreshUpdateList( QString insteadBinary, bool automatically ) {
    qDebug( "fuuuuuuuuck" );

    // local versions of launcher
    localLauncherVersion = LAUNCHER_VERSION;
    localInsteadVersion = detectInsteadVersion(insteadBinary);
    // retrieve remote versions
    m_automatically = automatically;
    setHtml( "<h3>" + tr("Loading updates ... please wait") + "</h3>" );
    QUrl url(SW_UPDATE_URL);
    qDebug() << "Downloading update list from " << url.toString();
    m_listServer->setHost(url.host());
    setEnabled( false );
    m_listServer->get(url.path());
    m_listLoadProgress->setValue(0);
}

void UpdateWidget::listServerDone( bool error ) {
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

void UpdateWidget::parseUpdateList( QXmlStreamReader *xml ) {
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

void UpdateWidget::generateUpdateMessage() {
    qDebug() << "Instead version " << remoteInsteadVersion << "at url" << urlInstead;
    qDebug() << "Launcher version " << remoteLauncherVersion << "at url" << urlLauncher;
    qDebug() << "Local instead version " << localInsteadVersion;
    qDebug() << "Local launcher version " << localLauncherVersion;

    bool needUpdateInstead = false;
    bool needUpdateLauncher = false;

    QString text;

    if ( compareVersions( remoteInsteadVersion, localInsteadVersion) == 1 ) {
        if ( localInsteadVersion == "0" ) {
            text += "<h3>" + tr("Instead update to ") + remoteInsteadVersion + "</h3>";
        } else {
            text += "<h3>" + tr("Instead update from ") + localInsteadVersion + tr(" to ") + remoteInsteadVersion + "</h3>";
        }
        text += "<a href=\"" + urlInstead + "\">" + urlInstead + "</a>";
        needUpdateInstead = true;
    }
    if ( compareVersions( remoteLauncherVersion, localLauncherVersion) == 1 ) {
        text += "<h3>" + tr("Launcher update from ") + localLauncherVersion + tr(" to ") + remoteLauncherVersion + "</h3>";
        text += "<a href=\"" + urlLauncher + "\">" + urlLauncher + "</a>";
        needUpdateLauncher = true;
    }

    if (!needUpdateInstead && !needUpdateLauncher) {
        // auto close window if no updates available
        if ( m_automatically ) {
            hide();
            return;
        }
        text += "<h3>"+tr("No updates available")+"</h3>";
    }

    setHtml( text );

    if ( m_automatically ) show();

}

void UpdateWidget::checkUpdates( QWidget *parent, QString insteadBinary, bool automatically ) {
    qDebug( "CCCCCHHHHHHHHHHH" );
//    UpdateWidget *window = new UpdateWidget(parent);
//    window->setAttribute(Qt::WA_DeleteOnClose, true);
//    window->setWindowModality(Qt::WindowModal);
    if ( !automatically ) show();
    refreshUpdateList( insteadBinary, automatically );
}

QString UpdateWidget::detectInsteadVersion( QString insteadBinary ) {
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
