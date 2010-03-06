#include "mainwindow.h"
#include "updatewidget.h"
#include "ui_mainwindow.h"
#include "qunzip.h"
#include "platform.h"
#include <QSettings>
#include "config.h"
#include "global.h"

class NetGameItem: public QTreeWidgetItem {
    public:
	NetGameItem() {
	    setTextAlignment( 1, Qt::AlignCenter );
            setTextAlignment( 2, Qt::AlignCenter );
	}
	
	NetGameItem( QTreeWidget *parent ) : QTreeWidgetItem( parent, QTreeWidgetItem::Type ) {
	    setTextAlignment( 1, Qt::AlignCenter );
	    setTextAlignment( 2, Qt::AlignCenter );
	}
	
	~NetGameItem() {
	}

	void setInfo( const NetGameInfo &info ) {
	    m_info = info;
	    setText( 0, info.title() );
	    setText( 1, info.version() );
	}

	NetGameInfo info() {
	    return m_info;
	}

    private:
	NetGameInfo m_info;
};

class LocalGameItem: public QTreeWidgetItem {
    public:
	LocalGameItem() {
	    setTextAlignment( 1, Qt::AlignCenter );
	}
	
	LocalGameItem( QTreeWidget *parent ) : QTreeWidgetItem( parent, QTreeWidgetItem::Type ) {
	    setTextAlignment( 1, Qt::AlignCenter );
	}
	
	~LocalGameItem() {
	}

	void setInfo( const GameInfo &info ) {
	    m_info = info;
	    setText( 0, info.title() );
	    setText( 1, info.version() );
	}

	GameInfo info() {
	    return m_info;
	}

    private:
	GameInfo m_info;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_ui(new Ui::MainWindow)
{
    listIsDirty = false;
    QString langSuffix = QLocale().name().split( "_" ).first();
    QTranslator *translator = new QTranslator( this );
    if ( translator->load( ":/instead-launcher_" + langSuffix + ".qm" ) )
	QApplication::installTranslator( translator );
    else qDebug() << "can't find instead-launcher_" + langSuffix + ".qm";

    QString translationsPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    qDebug() << "Qt translations path " << translationsPath;
    QTranslator *sysTranslator = new QTranslator( this );
    if ( sysTranslator->load( translationsPath + "/qt_" + langSuffix + ".qm" ) ) {
        QApplication::installTranslator( sysTranslator );
        qDebug() << "Used Qt translation from " << translationsPath;
    } else if ( sysTranslator->load( "qt_" + langSuffix + ".qm" ) ) {
        QApplication::installTranslator( sysTranslator );
        qDebug() << "Used Qt translation from local directory";
    } else {
        qDebug() << "Can't find qt_" + langSuffix + ".qm";
    }

    m_ui->setupUi(this);

    m_ui->listGames->header()->setResizeMode( 0, QHeaderView::Interactive );
    m_ui->listGames->header()->setResizeMode( 1, QHeaderView::ResizeToContents );
    m_ui->listGames->header()->setDefaultAlignment( Qt::AlignHCenter );
    m_ui->listNewGames->header()->setResizeMode( 0, QHeaderView::Interactive );
    m_ui->listNewGames->header()->setResizeMode( 1, QHeaderView::Fixed );
    m_ui->listNewGames->header()->setResizeMode( 2, QHeaderView::ResizeToContents );
    m_ui->listNewGames->headerItem()->setTextAlignment( 1, Qt::AlignHCenter );
    m_ui->listNewGames->headerItem()->setTextAlignment( 2, Qt::AlignHCenter );
    m_ui->proxyPasswordLineEdit->setEchoMode( QLineEdit::Password );

    m_ui->listGames->sortByColumn(0, Qt::AscendingOrder); /* added by Peter */
   /* m_ui->listNewGames->sortByColumn(0, Qt::AscendingOrder); */ /*new games are sorted by repo author! */

    m_ui->listGames->setAlternatingRowColors( true );
    m_ui->listNewGames->setAlternatingRowColors( true );

    setWindowTitle( "INSTEAD LAUNCHER - "LAUNCHER_VERSION );
    setWindowIcon( QIcon( ":/resources/icon.png" ) );

    resetConfig();
    loadConfig();

    refreshLocalGameList();

    m_listServer = new QHttp(this);
    connect( m_listServer, SIGNAL( done( bool ) ), this, SLOT( listServerDone( bool ) ) );

    m_listLoadProgress = new QProgressDialog(parent);
    m_listLoadProgress->setLabelText( tr( "Game list downloading" ) + "..." );
    m_listLoadProgress->setWindowIcon( QIcon( ":/resources/icon.png" ) );
    connect( m_listLoadProgress, SIGNAL(canceled()), m_listServer, SLOT(abort()));
    connect( m_listServer, SIGNAL( dataReadProgress( int, int ) ), m_listLoadProgress, SLOT( setValue( int ) ) );

    m_gameServer = new QHttp( this );
    connect( m_gameServer, SIGNAL( done(bool ) ), this, SLOT( gameServerDone( bool ) ) );
    connect( m_gameServer, SIGNAL( responseHeaderReceived( QHttpResponseHeader ) ), this, SLOT( gameServerResponseHeaderReceived( QHttpResponseHeader ) ) );
    m_gameLoadProgress = new QProgressDialog(parent);
    m_gameLoadProgress->setWindowIcon( QIcon( ":/resources/icon.png" ) );
    connect( m_gameServer, SIGNAL( dataReadProgress( int, int ) ), m_gameLoadProgress, SLOT( setValue( int ) ) );
    connect( m_gameLoadProgress, SIGNAL(canceled()), m_gameServer, SLOT(abort()));

    connect( m_ui->installPushButton, SIGNAL( clicked() ), this, SLOT( installSelectedGame() ) );
    connect( m_ui->refreshPushButton, SIGNAL( clicked() ), this, SLOT( refreshNetGameList() ) );
    connect( m_ui->addSourcePushButton, SIGNAL( clicked() ), this, SLOT( addSourcePushButtonClicked() ) );
    connect( m_ui->deleteSourcePushButton, SIGNAL( clicked() ), this, SLOT( deleteSourcePushButtonClicked() ) );
    connect( m_ui->playPushButton, SIGNAL( clicked() ), this, SLOT( playSelectedGame() ) );
    connect( m_ui->resetPushButton, SIGNAL( clicked() ), this, SLOT( resetPushButtonClicked() ) );
    connect( m_ui->langComboBox, SIGNAL( activated( int ) ), this, SLOT( refreshNetGameList() ) );
    connect( m_ui->listGames, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ), this, SLOT( playSelectedGame() ) );
    connect( m_ui->browseInsteadPath, SIGNAL( clicked() ), this, SLOT( browseInsteadPath() ) );
    connect( m_ui->browseGamesPath, SIGNAL( clicked() ), this, SLOT( browseGamesPath() ) );
    connect( m_ui->removePushButton, SIGNAL( clicked() ), this, SLOT( removeSelectedGame() ) );
    connect( m_ui->listNewGames, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ), this, SLOT( installSelectedGame() ) );

    connect( m_ui->proxyServerLineEdit, SIGNAL( textChanged(const QString &) ), this, SLOT( updateProxy() ) );
    connect( m_ui->proxyPortLineEdit, SIGNAL( textChanged(const QString &) ), this, SLOT( updateProxy() ) );
    connect( m_ui->proxyUserLineEdit, SIGNAL( textChanged(const QString &) ), this, SLOT( updateProxy() ) );
    connect( m_ui->proxyPasswordLineEdit, SIGNAL( textChanged(const QString &) ), this, SLOT( updateProxy() ) );
    connect( m_ui->proxyGroupBox, SIGNAL( clicked( bool ) ), this, SLOT( updateProxy() ) );

    connect( m_ui->gamesDir, SIGNAL(textChanged ( const QString &)), this, SLOT(gamesDirChanged()) );
    connect( m_ui->tabWidget, SIGNAL(currentChanged ( int )), this, SLOT(tabChanged(int)) );

    connect( m_ui->checkUpdatesButton, SIGNAL( clicked() ), this, SLOT( checkUpdates() ) );

    if (m_ui->autoRefreshCheckBox->isChecked()) {
        refreshNetGameList();
    }

    updateProxy();

    m_swUpdateWidget = new UpdateWidget( this );
    m_ui->swUpdateLayout->addWidget( m_swUpdateWidget );
    if (m_ui->autoRefreshSwCheckBox->isChecked()) {
        m_swUpdateWidget->checkUpdates( this, m_ui->lineInsteadPath->text(), true );
    }

    if ( !QFile::exists( m_ui->lineInsteadPath->text() ) ) {
	show();
	update();
	m_ui->tabWidget->setCurrentIndex( 2 );
	m_ui->lineInsteadPath->setFocus();
	QMessageBox::warning( this, tr( "INSTEAD was not found" ), tr( "Please input the proper INSTEAD path" ) + "." );
    }

/*
    if (m_ui->autoRefreshSwCheckBox->isChecked()) {
        UpdateWindow::checkUpdates( this, m_ui->lineInsteadPath->text(), true );
    }
*/
}

MainWindow::~MainWindow()
{
    saveConfig();
    delete m_ui;
}

void MainWindow::removeSelectedGame()
{
    LocalGameItem *item = ( LocalGameItem * )m_ui->listGames->currentItem();

    if ( !item ) {
	return;
    }

    QString gameName = item->info().name();
    QString gamesPath = m_ui->gamesDir->text();

    QString message = QString( "%1 \"%2\"?" ).arg( tr( "Do you really want to remove game" ) ).arg( item->info().title() );
    if ( QMessageBox::question( this, tr( "Confirm" ), message, QMessageBox::Yes, QMessageBox::No ) != QMessageBox::Yes ) {
	return;
    }

    QString path = QString( gamesPath + "/" + gameName ).replace( "//", "/" );

    if ( !QDir( path ).exists() ) {
	QMessageBox::critical( this, tr( "Fail" ), tr( "Nothing to done" ) );
	return;
    }

    qWarning() << "removing game " << gameName + " at " + path;

    if ( QFileInfo( path ).isSymLink() ) {
	QFile::remove( path );
    } else {
	QStack<QString> m_pathStack;
	m_pathStack.push( path );

	while( m_pathStack.count() ) {
	    QDir dir( m_pathStack.pop() );
	    if ( !dir.exists() ) {
		continue;
	    }
	    if ( QDir().rmpath( dir.absolutePath() ) ) {
		qWarning() << "removed: " << dir.absolutePath();
		continue;
	    } else m_pathStack.push( dir.absolutePath() );
	    QFileInfoList infos = dir.entryInfoList( QDir::AllEntries | QDir::NoDotAndDotDot );
	    QFileInfoList::Iterator infoIt = infos.begin();
	    while( infoIt != infos.end() ) {
		if ( ( *infoIt ).isDir() ) {
		    m_pathStack.push( dir.absolutePath() + "/" + ( *infoIt ).fileName() );
		    break;
		} else {
		    QString filePath = ( *infoIt ).absolutePath() + "/" + ( *infoIt ).fileName();
		    if( !QFile( ( *infoIt ).absolutePath() + "/" + ( *infoIt ).fileName() ).remove() ) {
			qWarning() << "can't remove file: " << filePath;
			return;
		    }
		    qWarning() << "removed: " << filePath;
		}
		infoIt++;
	    }
	}
    }

    refreshLocalGameList();
    refreshNetGameList();

    QMessageBox::information( this, tr( "Success" ), tr( "The game has successfully removed" ) );
}

void MainWindow::playSelectedGame()
{
    if (!m_ui->listGames->currentItem()) return;
    LocalGameItem *item = static_cast<LocalGameItem *>(m_ui->listGames->currentItem());
    QString gameName = item->info().name();
    QString insteadPath = m_ui->lineInsteadPath->text();
    QString gamesPath = m_ui->gamesDir->text();
    if (gamesPath.right(1) != QDir::separator()) gamesPath += QDir::separator();
    m_process = new QProcess();

    QStringList arguments;

    QFileInfo fileInfo(insteadPath);
    m_process->setWorkingDirectory(fileInfo.path());

    arguments << "-game" << gameName;
    arguments << "-nostdgames";
    arguments << "-gamespath" << gamesPath;

    if ( !m_ui->insteadParameters->text().isEmpty() ) {
        arguments.append( m_ui->insteadParameters->text().split(" ",  QString::SkipEmptyParts) );
    }

    qDebug() << "Launching " << insteadPath << arguments;

    connect( m_process, SIGNAL(started()), this, SLOT( processStarted()) );
    connect( m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT( processError(QProcess::ProcessError)) );
    connect( m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT( processFinished(int, QProcess::ExitStatus)) );

    m_process->start(insteadPath, arguments); // may startDetached be better? :)

}

void MainWindow::detailsLinkClicked( const QString &link ) 
{
    QDesktopServices::openUrl( link );
}

void MainWindow::browseInsteadPath() {
    QString fileName = QDir::toNativeSeparators(QFileDialog::getOpenFileName(this));
    if (!fileName.isEmpty()) m_ui->lineInsteadPath->setText(fileName);
}

void MainWindow::browseGamesPath() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Games directory"), "", QFileDialog::ShowDirsOnly);
    dir = QDir::toNativeSeparators(dir);
    if (!dir.isEmpty()) m_ui->gamesDir->setText(dir);
}

void MainWindow::processStarted() {
    qDebug() << "Succesfully launched";
    hide();
}

void MainWindow::processError( QProcess::ProcessError error) {
    m_process->deleteLater();
    qDebug() << "Creation error ";
    QMessageBox::critical(this, tr( "Can't run the game" ), tr( "Make sure that INSTEAD has been installed" ) + "." );
}

void MainWindow::processFinished( int exitCode, QProcess::ExitStatus exitStatus ) {
    m_process->deleteLater();
    qDebug() << "Game closed";
    show();
}

void MainWindow::refreshNetGameList(bool next) {
    static uint currentIdx=0;
    if(!next) {
        currentIdx=0; // in first row we always have official list url
        m_ui->listNewGames->clear();
    }
    QListWidgetItem * currentItem=m_ui->updateUrlList->item(currentIdx);
    if(currentItem!=NULL) {
        const QString currentUrl = currentItem->text();
        currentIdx++;
        qDebug() << "Updating list from " << currentUrl;
        QUrl url(currentUrl);
	m_listServer->setProxy( *Global::ptr()->networkProxy() );
        m_listServer->setHost(url.host());
        setEnabled( false );
        m_listServer->get(url.path());
//        m_listLoadProgress->setMinimumDuration(2000);
        m_listLoadProgress->setValue(0);
    }
}

void MainWindow::installSelectedGame() {
    m_ui->installPushButton->setDisabled(true);
    if ( !downloadGame(m_ui->listNewGames->currentItem()) ) {
        m_ui->installPushButton->setDisabled( false );
    }
}

void MainWindow::listServerDone(bool error) {
    qDebug( "List has been downloaded" );

    setEnabled( true );
    m_listLoadProgress->reset();
    if(!error) {
	QByteArray ba = m_listServer->readAll();
	qDebug( "ALL LIST: %s", ba.data() );
        QXmlStreamReader xml( ba );
        while (!xml.atEnd()) {
            xml.readNext();
            if (xml.isStartElement() && xml.name() == "game_list" && xml.attributes().value( "version" ) == "1.0" ) {
                parseGameList(&xml);
                break; // it should be only one game list
            }
        }
        if (xml.hasError()) {
            qWarning("Warning: errors while parsing game list");
            const QString s = xml.errorString();
            qWarning("%s", s.toLocal8Bit().data());
        }
        m_ui->listNewGames->resizeColumnToContents(0);
        refreshNetGameList(true); //load next game list if needed
    }
    else {
	QMessageBox::critical( this, tr( "Error" ), tr( "Can't retrieve game list. If you use proxy connection, check the proxy settings." ) );
        qWarning("WARN: errors while downloading");
    }
}

void MainWindow::parseGameList( QXmlStreamReader *xml )
{
    Q_ASSERT(xml->name() == "game_list");
    while (!xml->atEnd()) {
        xml->readNext();
        if ( xml->isStartElement() && xml->name() == "game" ) {
            qDebug("A game in the list!");
            parseGameInfo( xml );
        }
    }
}

void MainWindow::parseGameInfo( QXmlStreamReader *xml ) {
    Q_ASSERT( xml->name() == "game" );
    NetGameInfo info;
    QStringList depends;
    while( !xml->atEnd() ) {
        xml->readNext();
        if ( xml->isStartElement() ) {
	    if( xml->name() == "name" )
        	info.setName( xml->readElementText() );
            else if( xml->name() == "title" )
        	info.setTitle( xml->readElementText() );
            else if(xml->name() == "version")
        	info.setVersion( xml->readElementText() );
            else if( xml->name() == "url" )
        	info.setUrl( xml->readElementText() );
            else if( xml->name() == "md5" )
        	info.setMD5( xml->readElementText() );
            else if( xml->name() == "instead" )
        	info.setInstead( xml->readElementText() );
            else if( xml->name() == "lang" )
        	info.setLang( xml->readElementText() );
            else if( xml->name() == "descurl" )
        	info.setDescUrl( xml->readElementText() );
            else if( xml->name() == "depend" )
        	depends.append( xml->readElementText() );
        }
        if( xml->isEndElement() && xml->name()=="game" )
            break;
    }
    info.setDepends( depends );
    // проверяем что такой же версии игры нет в локальном списке
    if ( !hasLocalGame( info ) && ( m_ui->langComboBox->currentText() == tr( "all" ) || info.lang() == m_ui->langComboBox->currentText() ) ) {
	qDebug( "Adding game to the list %s", info.title().toLocal8Bit().data() );
	NetGameItem *game = new NetGameItem( m_ui->listNewGames );
	game->setInfo( info );

	QLabel *detailsLinkLabel = new QLabel( "<a href=" + info.descUrl() + ">" + info.descUrl() + "</a>", this );
	detailsLinkLabel->setAlignment( Qt::AlignLeft );
	connect( detailsLinkLabel, SIGNAL( linkActivated( const QString & ) ), this, SLOT( detailsLinkClicked( const QString & ) ) );
	m_ui->listNewGames->setItemWidget( game, 2, detailsLinkLabel );
    }
}

bool MainWindow::downloadGame( QTreeWidgetItem *game ) {
    if ( !game )
	return false;
    QList<QTreeWidgetItem *> list = findEssentialGames( game );
    if ( list.count() ) {
	QList<QTreeWidgetItem *>::Iterator it = list.begin();
	QStringList games;
	while( it != list.end() ) {
	    games.append( QString( "\"%1\"" ).arg( ( ( NetGameItem * )( *it ) )->info().title() ) );
	    it++;
	}
	QMessageBox::critical( this, tr( "This game depends on the other ones" ), tr( "You have to install the following games before this one" ) + ": " + games.join( ", " ) );
	return false;
    }
    m_gameFile = new QTemporaryFile();
    QUrl url( ( ( NetGameItem * )game )->info().url() );
    m_gameServer->setHost( url.host() );
    m_gameServer->setProxy( *Global::ptr()->networkProxy() );
    setEnabled( false );
    m_gameServer->get( url.path(), m_gameFile );
    m_downloadingFileName = url.path().split( "/" ).last();
    m_gameLoadProgress->setLabelText( QString( "%1 \"%2\"..." ).arg( tr( "Game downloading" ) ).arg( ( ( NetGameItem *)game )->info().title() ) );
    m_gameLoadProgress->setValue(0);
    return true;
}

void MainWindow::gameServerResponseHeaderReceived ( const QHttpResponseHeader & resp ) {
    qDebug("Header received. LEN=%d", resp.contentLength());
    m_gameLoadProgress->setMaximum(resp.contentLength());
}

void MainWindow::gameServerDone( bool error ) {
    setEnabled( true );
    m_gameLoadProgress->reset();
    m_ui->installPushButton->setEnabled(true);
    if(!error){
        QString games_dir = m_ui->gamesDir->text();
        if (games_dir.right(1) != QDir::separator()) games_dir += QDir::separator();
        if (!checkOrCreateGameDir(games_dir)) {
            qWarning() << "Can't create games directory";
            QMessageBox::critical(this, tr( "Error" ), tr( "Can't create dir" ) + ": " + games_dir);
            m_gameFile->close();
            delete m_gameFile;
            return;
        }
        QString arch_name = games_dir + m_downloadingFileName;
        if ( QFile::exists( arch_name ) ) {
	    QFile::remove( arch_name );
        }
        if ( !m_gameFile->copy( arch_name ) ) {
            qCritical() << "can't copy temporary file to the game dir: " << arch_name;
            m_gameFile->close();
            delete m_gameFile;
            return;
        }
	bool unzipped = true;
        if ( !qUnzip( arch_name, games_dir ) ) {
	    unzipped = false;
            qCritical() << "can't unzip game: " << arch_name;
        }
        if ( !QFile::remove( arch_name ) ) {
	    qCritical() << "can't remove temporary file: " << arch_name;
        }
        if ( !unzipped ) {
            m_gameFile->close();
            delete m_gameFile;
            return;
        }
        QMessageBox::information( this, tr( "Success" ), tr( "The game has been downloaded and unpacked" ) );
	refreshLocalGameList();
	refreshNetGameList();
    }
    else {
        QMessageBox::critical( this, tr( "Error" ), tr( "Can't download the game. If you use proxy connection, check the proxy settings." ) );
        qWarning("WARN: Game load error");
        qWarning()<<QHttp().errorString();
    }
    m_gameFile->close();
    delete m_gameFile;
}

// try to find all essential games for the selected one
QList<QTreeWidgetItem *> MainWindow::findEssentialGames( QTreeWidgetItem *item ) {
    QList<QTreeWidgetItem *> essential;
    if ( !item ) {
	return essential;
    }
    for( int i = 0; i < m_ui->listNewGames->topLevelItemCount(); i++ ) {
	if ( ( ( NetGameItem * )item )->info().depends().contains( ( ( NetGameItem * )m_ui->listNewGames->topLevelItem( i ) )->info().name() ) ) {
	    essential.append( m_ui->listNewGames->topLevelItem( i ) );
	}
    }
    return essential;
}

// Чтение инфы об игре из main.lua
bool MainWindow::getLocalGameInfo(const QDir gameDir, const QString gameID, GameInfo &info) {

    qDebug() << "Analyzing subdir: " << gameID;

    // проверяем наличие файлика main.lua
    if (!gameDir.exists(gameID + "/main.lua")) {
        qWarning() << "main.lua not found - not a game";
        return false;
    }

    // читаем main.lua
    QFile file(gameDir.absolutePath() + "/" + gameID + "/main.lua");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Can't open " << file.fileName();
        return false;
    }
    QTextStream in(&file);

    QString namePrefix = "-- $Name:";
    QString versionPrefix = "-- $Version:";
    QString name = gameID;
    QString version = "";
    bool hasVersion = false, hasName = false;
    while (!hasVersion || !hasName) {
        QString line = in.readLine().simplified();
        if ( line.isNull() || !line.startsWith("--") ) break;
        if ( line.startsWith( namePrefix ) ) {
            name = line.mid( namePrefix.length() ).trimmed();
            if ( name.endsWith('$') ) name = name.left( name.length() - 1 ).trimmed();
            hasName = true;
        } else if ( line.startsWith( versionPrefix ) ) {
            version = line.mid( versionPrefix.length() ).trimmed();
            if ( version.endsWith('$') ) version = version.left( version.length() - 1 ).trimmed();
            hasVersion = true;
        }
    }

    if (!hasName) qWarning() << "Game doesn't have $Name tag!";
    if (!hasVersion) qWarning() << "Game doesn't have $Version tag!";

    info.setName( gameID );
    info.setTitle( name );
    info.setVersion( version );

    return true;
}

bool MainWindow::hasLocalGame( const GameInfo &info ) {
    for( int i = 0; i < m_ui->listGames->topLevelItemCount(); i++ ) {
	LocalGameItem *item = ( ( LocalGameItem * )m_ui->listGames->topLevelItem( i ) );
	if ( item->info() == info )
	    return true;
    }
    return false;
}

// Обновление списка установленных игр
void MainWindow::refreshLocalGameList() {

    // очищаем список
    m_ui->listGames->clear();

    // получаем директорию с играми
    QString gamePath = m_ui->gamesDir->text();
    QDir gameDir(gamePath);
    qDebug() << "game path: " << gamePath;

    // now we don't create directory and silently return
    if (!gameDir.exists()) {
        listIsDirty = false;
        return;
    }

    /*if (!checkOrCreateGameDir(gamePath)) {
        qWarning() << "Can't create games directory";
        QMessageBox::critical(this, tr( "Error" ), tr( "Can't create dir" ) + ": " + gamePath);
        return;
    }*/

    // просматриваем все подкаталоги
    QStringList gameList = gameDir.entryList( QDir::AllDirs|QDir::NoDotAndDotDot, QDir::NoSort );
    QString gameID;
    foreach( gameID, gameList ) {
        GameInfo info;
        if ( getLocalGameInfo( gameDir, gameID, info ) ) {
            // TODO добавляем игру в список
            qDebug() << "found game " << info.name() << info.title() << info.version();
            LocalGameItem *game = new LocalGameItem( m_ui->listGames );
            game->setInfo( info );
        }
    }
    m_ui->listGames->resizeColumnToContents(0);
    listIsDirty = false;
}

void MainWindow::resetConfig() {
    m_ui->lineInsteadPath->setText( getDefaultInterpreterPath() );
    m_ui->autoRefreshCheckBox->setChecked(false);
    m_ui->autoRefreshSwCheckBox->setChecked(false);
    m_ui->updateUrlList->clear();
    m_ui->updateUrlList->addItem( GAMES_UPDATE_URL );
    QListWidgetItem *item = m_ui->updateUrlList->item(0);
    item->setFlags(item->flags() & ~ (Qt::ItemIsEnabled));
    m_ui->gamesDir->setText( getGameDirPath() );
    m_ui->insteadParameters->setText("");
    m_ui->proxyServerLineEdit->setText( "127.0.0.1" );
    m_ui->proxyPortLineEdit->setText( "3128" );
    m_ui->proxyUserLineEdit->setText( QString() );
    m_ui->proxyPasswordLineEdit->setText( QString() );
    m_ui->proxyAuthGroupBox->setChecked( false );
    m_ui->proxyGroupBox->setChecked( false );
}

void MainWindow::loadConfig() {
    QSettings conf(getConfigPath(), QSettings::IniFormat);
    QString insteadPath = conf.value("InsteadPath", getDefaultInterpreterPath()).toString();
    bool autoRefresh = conf.value("AutoRefresh", "false").toString() == "true";
    bool autoRefreshSW = conf.value("AutoRefreshSW", "false").toString() == "true";
    QString lang = conf.value( "Language", "*" ).toString();
    QString gamesDir = conf.value("GamesPath", getGameDirPath()).toString();
    QString insteadParameters = conf.value("InsteadParameters", "").toString();

    m_ui->lineInsteadPath->setText(insteadPath);
    m_ui->autoRefreshCheckBox->setChecked(autoRefresh);
    m_ui->autoRefreshSwCheckBox->setChecked(autoRefreshSW);
    int index = m_ui->langComboBox->findText( ( lang == "*" ) ? tr( "all" ) : lang );
    m_ui->langComboBox->blockSignals( true );
    m_ui->langComboBox->setCurrentIndex( index );
    m_ui->langComboBox->blockSignals( false );
    m_ui->updateUrlList->clear();
    m_ui->tabWidget->setCurrentIndex( conf.value( "LastTabIndex", 0 ).toInt() );
    conf.beginGroup( "Proxy" );
	m_ui->proxyServerLineEdit->setText( conf.value( "Server", "127.0.0.1" ).toString() );
	m_ui->proxyPortLineEdit->setText( conf.value( "Port", "3128" ).toString() );
	conf.beginGroup( "Authentication" );
	    m_ui->proxyUserLineEdit->setText( conf.value( "User", "" ).toString() );
	    m_ui->proxyPasswordLineEdit->setText( conf.value( "Password", "" ).toString() );
	    m_ui->proxyAuthGroupBox->setChecked( conf.value( "Enabled", "" ).toBool() );
	conf.endGroup();
	m_ui->proxyGroupBox->setChecked( conf.value( "Enabled", false ).toBool() );
    conf.endGroup();
    conf.beginGroup( "MainWindow" );
	resize( conf.value( "Width", 500 ).toInt(), conf.value( "Height", 350 ).toInt() );
    conf.endGroup( );
    int count = conf.beginReadArray("UpdateURLs");
    if ( !count ) {
        m_ui->updateUrlList->addItem( GAMES_UPDATE_URL );
        QListWidgetItem *item = m_ui->updateUrlList->item(0);
        item->setFlags(item->flags() & ~ (Qt::ItemIsEnabled));
    } else {
	for (int i=0;i<count;i++) {
    	    conf.setArrayIndex(i);
    	    m_ui->updateUrlList->addItem(conf.value("URL", "").toString());
    	    if (i==0) {
        	QListWidgetItem *item = m_ui->updateUrlList->item(i);
        	item->setFlags(item->flags() & ~ (Qt::ItemIsEnabled));
    	    }
	}
    }
    m_ui->gamesDir->setText(gamesDir);
    m_ui->insteadParameters->setText(insteadParameters);
    conf.endArray();
    qDebug() << "Config loaded";
}


void MainWindow::saveConfig() {
    QSettings conf(getConfigPath(), QSettings::IniFormat);
    conf.setValue("InsteadPath", m_ui->lineInsteadPath->text());
    conf.setValue("AutoRefresh", (m_ui->autoRefreshCheckBox->isChecked() ? "true" : "false"));
    conf.setValue("AutoRefreshSW", (m_ui->autoRefreshSwCheckBox->isChecked() ? "true" : "false"));
    conf.setValue("Language", ( m_ui->langComboBox->currentText() == tr( "all" ) ) ? "*" : m_ui->langComboBox->currentText() );
    conf.setValue( "LastTabIndex", m_ui->tabWidget->currentIndex() );
    conf.beginGroup( "Proxy" );
	conf.setValue( "Server", m_ui->proxyServerLineEdit->text() );
	conf.setValue( "Port", m_ui->proxyPortLineEdit->text() );
	conf.beginGroup( "Authentication" );
	    conf.setValue( "User", m_ui->proxyUserLineEdit->text() );
	    conf.setValue( "Password", m_ui->proxyPasswordLineEdit->text() );
	    conf.setValue( "Enabled", m_ui->proxyAuthGroupBox->isChecked() );
	conf.endGroup();
	conf.setValue( "Enabled", m_ui->proxyGroupBox->isChecked() );
    conf.endGroup();
    conf.beginGroup( "MainWindow" );
	conf.setValue( "Width", width() );
	conf.setValue( "Height", height() );
    conf.endGroup();
    conf.beginWriteArray("UpdateURLs", m_ui->updateUrlList->count());
    for (int i=0;i<m_ui->updateUrlList->count();i++) {
        conf.setArrayIndex(i);
        conf.setValue("URL", m_ui->updateUrlList->item(i)->text());
    }
    conf.endArray();
    conf.setValue("GamesPath", m_ui->gamesDir->text());
    conf.setValue("InsteadParameters", m_ui->insteadParameters->text());

    qDebug() << "Config saved";
}

void MainWindow::resetPushButtonClicked() {
    if( QMessageBox::question( this, tr( "Reset settings" ), tr( "Are you sure?" ) ) == QMessageBox::No )
	return;
    resetConfig();
}

void MainWindow::gamesDirChanged() {
    listIsDirty = true;
    qDebug() << "set dirty flag";
}

void MainWindow::tabChanged(int pos) {
    if (listIsDirty && pos == 0) {
        qDebug() << "list dirty! reloading ...";
        refreshLocalGameList();
    }
}

void MainWindow::addSourcePushButtonClicked()
{
    QListWidgetItem * newItem = new QListWidgetItem( "http://" );
    uint newItemIdx = m_ui->updateUrlList->count();
    newItem->setFlags(Qt::ItemIsEditable|Qt::ItemIsSelectable|newItem->flags());
    m_ui->updateUrlList->insertItem(newItemIdx, newItem);
    m_ui->updateUrlList->editItem(m_ui->updateUrlList->item(newItemIdx));
}

void MainWindow::deleteSourcePushButtonClicked()
{
    int curr = m_ui->updateUrlList->currentRow();
    delete m_ui->updateUrlList->takeItem(curr);
}

void MainWindow::updateProxy()
{
    qDebug( "update proxy" );
    if ( m_ui->proxyGroupBox->isChecked() ) {
	qDebug( "proxy has switched on" );
        Global::ptr()->networkProxy()->setType( QNetworkProxy::HttpCachingProxy );
	Global::ptr()->networkProxy()->setHostName( m_ui->proxyServerLineEdit->text().simplified() );
        Global::ptr()->networkProxy()->setPort( m_ui->proxyPortLineEdit->text().simplified().toInt() );
        Global::ptr()->networkProxy()->setUser( m_ui->proxyUserLineEdit->text().simplified() );
        Global::ptr()->networkProxy()->setPassword( m_ui->proxyPasswordLineEdit->text() );
    } else {
	qDebug( "proxy has switched off" );
	Global::ptr()->networkProxy()->operator=( QNetworkProxy() );
    }
}

void MainWindow::checkUpdates() {
    m_swUpdateWidget->checkUpdates( this, m_ui->lineInsteadPath->text(), false );
}
