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
    // local versions
    localLauncherVersion = LAUNCHER_VERSION;
    localInsteadVersion = detectInsteadVersion( insteadBinary );
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
            // all right, generate message
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

    if (localInsteadVersion != remoteInsteadVersion) {
        text += "<h3>" + tr("Instead update from ") + localInsteadVersion + tr(" to ") + remoteInsteadVersion + "</h3>";
        text += "<a href=\"" + urlInstead + "\">" + urlInstead + "</a>";
        needUpdateInstead = true;
    }
    if (localLauncherVersion != remoteLauncherVersion) {
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
    qDebug() << "Instead binary at " << insteadBinary;
    return "1.0.5";
}
