#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qunzip.h"
#include <QSettings>

#define DEFAULT_UPDATE_URL "http://instead-launcher.googlecode.com/files/game_list.xml"

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
    if ( translator->load( "instead-launcher_" + langSuffix + ".qm" ) )
	QApplication::installTranslator( translator );

    m_ui->setupUi(this);

    m_ui->listGames->header()->setResizeMode( 0, QHeaderView::Interactive );
    m_ui->listGames->header()->setResizeMode( 1, QHeaderView::ResizeToContents );
    m_ui->listGames->header()->setDefaultAlignment( Qt::AlignHCenter );
    m_ui->listNewGames->header()->setResizeMode( 0, QHeaderView::Interactive );
    m_ui->listNewGames->header()->setResizeMode( 1, QHeaderView::Fixed );
    m_ui->listNewGames->header()->setResizeMode( 2, QHeaderView::ResizeToContents );
    m_ui->listNewGames->headerItem()->setTextAlignment( 1, Qt::AlignHCenter );
    m_ui->listNewGames->headerItem()->setTextAlignment( 2, Qt::AlignHCenter );

    m_ui->listGames->setAlternatingRowColors( true );
    m_ui->listNewGames->setAlternatingRowColors( true );

    setWindowTitle( "instead-launcher" );
    setWindowIcon( QIcon( ":/resources/icon.png" ) );

    resetConfig();
    loadConfig();

    refreshLocalGameList();

    m_listServer = new QHttp(this);
    connect( m_listServer, SIGNAL( done( bool ) ), this, SLOT( listServerDone( bool ) ) );

    m_listLoadProgress = new QProgressDialog(parent);
    m_listLoadProgress->setLabelText( tr( "Game list downloading" ) + "..." );
    connect( m_listLoadProgress, SIGNAL(canceled()), m_listServer, SLOT(abort()));
    connect( m_listServer, SIGNAL( dataReadProgress( int, int ) ), m_listLoadProgress, SLOT( setValue( int ) ) );

    m_gameServer = new QHttp( this );
    connect( m_gameServer, SIGNAL( done(bool ) ), this, SLOT( gameServerDone( bool ) ) );
    connect( m_gameServer, SIGNAL( responseHeaderReceived( QHttpResponseHeader ) ), this, SLOT( gameServerResponseHeaderReceived( QHttpResponseHeader ) ) );

    m_gameLoadProgress = new QProgressDialog(parent);
    connect( m_gameServer, SIGNAL( dataReadProgress( int, int ) ), m_gameLoadProgress, SLOT( setValue( int ) ) );
    connect( m_gameLoadProgress, SIGNAL(canceled()), m_gameServer, SLOT(abort()));

    connect( m_ui->installPushButton, SIGNAL( clicked() ), this, SLOT( installPushButtonClicked() ) );
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

    connect( m_ui->gamesDir, SIGNAL(textChanged ( const QString &)), this, SLOT(gamesDirChanged()) );
    connect( m_ui->tabWidget, SIGNAL(currentChanged ( int )), this, SLOT(tabChanged(int)) );

    if (m_ui->autoRefreshCheckBox->isChecked()) {
        refreshNetGameList();
    }
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

    refreshLocalGameList();
    refreshNetGameList();

    QMessageBox::information( this, tr( "Success" ), tr( "The game has successfully removed" ) );
}


void MainWindow::playSelectedGame() 
{
    LocalGameItem *item = static_cast<LocalGameItem *>(m_ui->listGames->currentItem());
    QString gameName = item->info().name();
    QString insteadPath = m_ui->lineInsteadPath->text();
    QString gamesPath = m_ui->gamesDir->text();
    if (gamesPath.right(1) != "/") gamesPath += "/";
    m_process = new QProcess();

#ifdef Q_OS_WIN
    QFileInfo fileInfo(insteadPath);
    m_process->setWorkingDirectory(fileInfo.path());
    qDebug() << QDir::toNativeSeparators(fileInfo.path());
    QString command = "\"" + insteadPath + "\" -game " + gameName;
    command += " -gamespath \"" + gamesPath + "\"";
#else
    QString command = insteadPath + " -game " + gameName;
    command += " -gamespath " + gamesPath;
#endif

    command += " -nostdgames";

    qDebug() << "Launching " << command;
    m_process->start(command); // may startDetached be better? :)

    connect( m_process, SIGNAL(started()), this, SLOT( processStarted()) );
    connect( m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT( processError(QProcess::ProcessError)) );
    connect( m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT( processFinished(int, QProcess::ExitStatus)) );
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
        qDebug() << "Updating list from " << currentUrl;// m_ui->lineUpdateUrl->text();
        QUrl url(currentUrl);
        m_listServer->setHost(url.host());
        setEnabled( false );
        m_listServer->get(url.path());
//        m_listLoadProgress->setMinimumDuration(2000);
        m_listLoadProgress->setValue(0);
    }
}

void MainWindow::installPushButtonClicked() {
    m_ui->installPushButton->setDisabled(true);
    downloadGame(m_ui->listNewGames->currentItem());
}

void MainWindow::listServerDone(bool error) {
    qDebug( "List has been downloaded" );

    setEnabled( true );
    m_listLoadProgress->reset();
    if(!error) {
        QXmlStreamReader xml( m_listServer->readAll() );
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
        }
        if( xml->isEndElement() && xml->name()=="game" )
            break;
    }
    // проверяем что такой же версии игры нет в локальном списке
    if ( !hasLocalGame( info ) && ( info.lang() == m_ui->langComboBox->currentText() ) ) {
	qDebug( "Adding game to the list %s", info.title().toLocal8Bit().data() );
	NetGameItem *game = new NetGameItem( m_ui->listNewGames );
	game->setInfo( info );
	
	QLabel *detailsLinkLabel = new QLabel( "<a href=" + info.descUrl() + ">" + tr( "open" ) +  " ..." + "</a>", this );
	detailsLinkLabel->setAlignment( Qt::AlignCenter );
	connect( detailsLinkLabel, SIGNAL( linkActivated( const QString & ) ), this, SLOT( detailsLinkClicked( const QString & ) ) );
	m_ui->listNewGames->setItemWidget( game, 2, detailsLinkLabel );
    }
}


void MainWindow::downloadGame( QTreeWidgetItem *game ) {
    Q_ASSERT( game != NULL );
    m_gameFile = new QTemporaryFile();
    QUrl url( ( ( NetGameItem * )game )->info().url() );
    m_gameServer->setHost( url.host() );
    setEnabled( false );
    m_gameServer->get( url.path(), m_gameFile );
    m_downloadingFileName = url.path().split( "/" ).last();
    m_gameLoadProgress->setLabelText( QString( "%1 \"%2\"..." ).arg( tr( "Game downloading" ) ).arg( ( ( NetGameItem *)game )->info().title() ) );
    m_gameLoadProgress->setValue(0);
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
        if (games_dir.right(1) != "/") games_dir += "/";
        QString arch_name = games_dir + m_downloadingFileName;
        if ( QFile::exists( arch_name ) ) {
	    QFile::remove( arch_name );
        }
        if ( !m_gameFile->copy( arch_name ) ) {
            qCritical() << "can't copy temporary file to the game dir: " << arch_name;
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
	    return;
        }
        QMessageBox::information( this, tr( "Success" ), tr( "The game has been downloaded and unpacked" ) );
	refreshLocalGameList();
	refreshNetGameList();
    }
    else {        
        qWarning("WARN: Game load error");
        qWarning()<<QHttp().errorString();
    }
    m_gameFile->close();
}


// Чтение инфы об игре из main.lua
bool MainWindow::getLocalGameInfo(const QDir gameDir, const QString gameID, GameInfo &info) {

    qDebug() << "Analyzing subdir: " << gameID;

    // проверяем наличие файлика main.lua
    if (!gameDir.exists(gameID + "/main.lua")) {
        qWarning() << "main.lua not found - not a game";
        return false;
    }

    // читаем первые две строчки main.lua
    QFile file(gameDir.absolutePath() + "/" + gameID + "/main.lua");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Can't open " << file.fileName();
        return false;
    }
    QTextStream in(&file);
    QString name = in.readLine();
    QString version = in.readLine();

    QRegExp regexName("-- \\$Name:(.*)\\$");
    if (!regexName.exactMatch(name)) {
        qWarning() << "First line doesn't contains game name";
        name = gameID;
    } else name = regexName.capturedTexts()[1].trimmed();


    QRegExp regexVersion("-- \\$Version:(.*)\\$");
    if (!regexVersion.exactMatch(version)) {
        qWarning() << "Second line doesn't contains game version";
    }
    version = regexVersion.capturedTexts()[1].trimmed();

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

    if (!checkOrCreateGameDir(gamePath)) {
        return;
    }

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
//    m_ui->lineUpdateUrl->setText( DEFAULT_UPDATE_URL ); //TODO: m_ui->updateUrlList
    m_ui->lineInsteadPath->setText( getDefaultInterpreterPath() );
    m_ui->autoRefreshCheckBox->setChecked(false);
    m_ui->updateUrlList->clear();
    m_ui->updateUrlList->addItem( DEFAULT_UPDATE_URL );
    QListWidgetItem *item = m_ui->updateUrlList->item(0);
    item->setFlags(item->flags() & ~ (Qt::ItemIsEnabled));
    m_ui->gamesDir->setText( getGameDirPath() );
}

void MainWindow::loadConfig() {
    QSettings conf(getConfigPath(), QSettings::IniFormat);
    QString insteadPath = conf.value("InsteadPath", getDefaultInterpreterPath()).toString();
    bool autoRefresh = conf.value("AutoRefresh", "false").toString() == "true";
    QString lang = conf.value("Language", "ru").toString(); // TODO: язык системы по дефолту; no! it's game languages
    QString gamesDir = conf.value("GamesPath", getGameDirPath()).toString();

    m_ui->lineInsteadPath->setText(insteadPath);
    m_ui->autoRefreshCheckBox->setChecked(autoRefresh);
    int index = m_ui->langComboBox->findText( lang );
    m_ui->langComboBox->blockSignals( true );
    m_ui->langComboBox->setCurrentIndex( index );
    m_ui->langComboBox->blockSignals( false );
    m_ui->updateUrlList->clear();
    m_ui->tabWidget->setCurrentIndex( conf.value( "LastTabIndex", 0 ).toInt() );
    conf.beginGroup( "MainWindow" );
	resize( conf.value( "Width", 500 ).toInt(), conf.value( "Height", 350 ).toInt() );
    conf.endGroup( );
    int count = conf.beginReadArray("UpdateURLs");
    if ( !count ) {
	m_ui->updateUrlList->addItem( DEFAULT_UPDATE_URL );
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
    conf.endArray();
    qDebug() << "Config loaded";
}


void MainWindow::saveConfig() {
    QSettings conf(getConfigPath(), QSettings::IniFormat);
    conf.setValue("InsteadPath", m_ui->lineInsteadPath->text());
    conf.setValue("AutoRefresh", (m_ui->autoRefreshCheckBox->isChecked() ? "true" : "false"));
    conf.setValue("Language", m_ui->langComboBox->currentText());
    conf.setValue( "LastTabIndex", m_ui->tabWidget->currentIndex() );
    conf.beginGroup( "MainWindow" );
	conf.setValue( "Width", width() );
	conf.setValue( "Height", height() );
    conf.endGroup( );
    conf.beginWriteArray("UpdateURLs", m_ui->updateUrlList->count());
    for (int i=0;i<m_ui->updateUrlList->count();i++) {
        conf.setArrayIndex(i);
        conf.setValue("URL", m_ui->updateUrlList->item(i)->text());
    }
    conf.endArray();
    conf.setValue("GamesPath", m_ui->gamesDir->text());

    qDebug() << "Config saved";
}

void MainWindow::resetPushButtonClicked() {
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


// Platform specific functions. Maybe move them into "platform.h" ?

bool MainWindow::checkOrCreateGameDir( QString gameDir ) {

    QDir dir(gameDir);
    if (dir.exists()) return true;

    qWarning() << "mkpath " << gameDir;

    if (!dir.mkpath( gameDir )) {
        QMessageBox::critical(this, tr( "Error" ), tr( "Can't create dir" ) + ": " + gameDir);
        qWarning() << "Can't create games directory";
        return false;
    }

    return true;
}

QString MainWindow::getGameDirPath() const
{
#ifdef Q_OS_UNIX
    return QDir::home().absolutePath() + "/.instead/games/";

#elif defined(Q_OS_WIN)
    return QDir::toNativeSeparators(QDir::home().absolutePath()) + "\\Local Settings\\Application Data\\instead\\games\\";

#else
#error "Unsupported OS"
#endif
}

QString MainWindow::getConfigPath() const
{
#ifdef Q_OS_UNIX
    return QDir::home().absolutePath() + "/.instead/launcher.ini";

#elif defined(Q_OS_WIN)
    return QDir::toNativeSeparators(QDir::home().absolutePath()) + "\\Local Settings\\Application Data\\instead\\launcher.ini";

#else
#error "Unsupported OS"
#endif
}

QString MainWindow::getDefaultInterpreterPath() const {
#ifdef Q_OS_UNIX
    return "/usr/local/bin/sdl-instead";

#elif defined(Q_OS_WIN)
    return "c:\\Program Files\\Pinebrush games\\INSTEAD\\sdl-instead.exe";

#else
#error "Unsupported OS"
#endif
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
