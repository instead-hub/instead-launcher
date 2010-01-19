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
	}
	
	LocalGameItem( QTreeWidget *parent ) : QTreeWidgetItem( parent, QTreeWidgetItem::Type ) {
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
    //m_ui->listNewGames->header()->setDefaultAlignment( Qt::AlignHCenter );

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
    connect( m_ui->playPushButton, SIGNAL( clicked() ), this, SLOT( playPushButtonClicked() ) );
    connect( m_ui->resetPushButton, SIGNAL( clicked() ), this, SLOT( resetPushButtonClicked() ) );
    connect( m_ui->langComboBox, SIGNAL( activated( int ) ), this, SLOT( refreshNetGameList() ) );

    if (m_ui->autoRefreshCheckBox->isChecked()) {
        refreshNetGameList();
    }
}

MainWindow::~MainWindow()
{
    saveConfig();
    delete m_ui;
}

void MainWindow::playPushButtonClicked() 
{
    LocalGameItem *item = static_cast<LocalGameItem *>(m_ui->listGames->currentItem());
    QString gameName = item->info().name();
    QString insteadPath = m_ui->lineInsteadPath->text();
    m_process = new QProcess();

#ifdef Q_OS_WIN
    QFileInfo fileInfo(insteadPath);
    m_process->setWorkingDirectory(fileInfo.path());
    qDebug() << QDir::toNativeSeparators(fileInfo.path());
    QString command = "\"" + insteadPath + "\" -game " + gameName;
#else
    QString command = insteadPath + " -game " + gameName;
#endif

    qDebug() << "Launching " << command;
    m_process->start(command); // may startDetached be better? :)

    connect( m_process, SIGNAL(started()), this, SLOT( processStarted()) );
    connect( m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT( processError(QProcess::ProcessError)) );
    connect( m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT( processFinished(int, QProcess::ExitStatus)) );
}

void MainWindow::detailsPushButtonClicked() 
{
    QString descUrl = ( ( QObject * )sender() )->property( "descurl" ).toString();
    QDesktopServices::openUrl( descUrl );
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
	
	QWidget *detailsWidget = new QWidget( m_ui->listNewGames );
	QHBoxLayout *detailsLayout = new QHBoxLayout( detailsWidget );
	detailsLayout->setContentsMargins( 0, 7, 0, 7 );
	QPushButton *detailsPushButton = new QPushButton( tr( "Open" ) + "...", detailsWidget );
	detailsPushButton->setMaximumWidth( 100 );
	connect( detailsPushButton, SIGNAL( clicked() ), this, SLOT( detailsPushButtonClicked() ) );
	detailsPushButton->setProperty( "descurl", info.descUrl() );
	detailsLayout->addWidget( detailsPushButton );
	m_ui->listNewGames->setItemWidget( game, 2, detailsWidget );
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
        QString games_dir = getGameDirPath();
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
    }
    name = regexName.capturedTexts()[1].trimmed();

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
    QString gamePath = getGameDirPath();
    QDir gameDir(gamePath);
    qDebug() << "game path: " << gamePath;

    if (!checkOrCreateGameDir()) {
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
    
}

void MainWindow::resetConfig() {
//    m_ui->lineUpdateUrl->setText( DEFAULT_UPDATE_URL ); //TODO: m_ui->updateUrlList
    m_ui->lineInsteadPath->setText( getDefaultInterpreterPath() );
    m_ui->autoRefreshCheckBox->setChecked(false);
}

void MainWindow::loadConfig() {
    QSettings conf(getConfigPath(), QSettings::IniFormat);
    QString insteadPath = conf.value("InsteadPath", getDefaultInterpreterPath()).toString();
    bool autoRefresh = conf.value("AutoRefresh", "false").toString() == "true";
    QString lang = conf.value("Language", "ru").toString(); // TODO: язык системы по дефолту

    m_ui->lineInsteadPath->setText(insteadPath);
    m_ui->autoRefreshCheckBox->setChecked(autoRefresh);
    int index = m_ui->langComboBox->findText( lang );
    m_ui->langComboBox->blockSignals( true );
    m_ui->langComboBox->setCurrentIndex( index );
    m_ui->langComboBox->blockSignals( false );
    /*
    QFile configFile(getConfigPath());
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Can't open config file. Don't worry :)";
        return;
    }
    QTextStream config(&configFile);
    QRegExp regex("(.*)=(.*)");
    while (!config.atEnd()) {
        QString line = config.readLine();
        if (!regex.exactMatch(line)) {
            qWarning() << "Syntax error in line " << line;
        } else {
            QString key = regex.capturedTexts()[1];
            QString value = regex.capturedTexts()[2];
            qDebug() << key << " = " << value;
            if (key == "UpdateURL") {
//                m_ui->lineUpdateUrl->setText(value); //TODO: m_ui->updateUrlList
            } else if (key == "InsteadPath") {
                m_ui->lineInsteadPath->setText(value);
            } else if (key == "AutoRefresh") {
                m_ui->autoRefreshCheckBox->setChecked( value=="true" );
            } else if (key == "Language") {
        	int index = m_ui->langComboBox->findText( value );
        	m_ui->langComboBox->blockSignals( true );
                m_ui->langComboBox->setCurrentIndex( index );
        	m_ui->langComboBox->blockSignals( false );
            }
        }
    }*/
    qDebug() << "Config loaded";
}


void MainWindow::saveConfig() {
    QSettings conf(getConfigPath(), QSettings::IniFormat);
    conf.setValue("InsteadPath", m_ui->lineInsteadPath->text());
    conf.setValue("AutoRefresh", (m_ui->autoRefreshCheckBox->isChecked() ? "true" : "false"));
    conf.setValue("Language", m_ui->langComboBox->currentText());
    /*
    QFile configFile(getConfigPath());
    if (!configFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Can't save config file!";
        return;
    }
    QTextStream config(&configFile);
//    config << "UpdateURL=" << m_ui->lineUpdateUrl->text() << endl; //TODO: m_ui->updateUrlList
    config << "InsteadPath=" << m_ui->lineInsteadPath->text() << endl;
    config << "AutoRefresh=" << (m_ui->autoRefreshCheckBox->isChecked() ? "true" : "false") << endl;
    config << "Language=" << m_ui->langComboBox->currentText() << endl;
    */
    qDebug() << "Config saved";
}

void MainWindow::resetPushButtonClicked() {
    resetConfig();
}


// Platform specific functions. Maybe move them into "platform.h" ?

bool MainWindow::checkOrCreateGameDir() {
    QDir gameDir(getGameDirPath());
    if ( gameDir.exists() ) return true;

    // если директория не существует, то создаем ее

    qDebug() << "created games directory";
    bool result = false;

#ifdef Q_OS_UNIX
    result = QDir::home().mkpath( ".instead/games" );

#elif defined(Q_OS_WIN)
    result = QDir::home().mkpath( "Local Settings\\Application Data\\instead\\games" );

#endif

    if (!result) {
        QMessageBox::critical(this, tr( "Error" ), tr( "Can't create dir" ) + ": " + gameDir.absolutePath());
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

void MainWindow::on_addListSource_clicked()
{
    QListWidgetItem * newItem = new QListWidgetItem("http://");
    uint newItemIdx = m_ui->updateUrlList->count();
    newItem->setFlags(Qt::ItemIsEditable|Qt::ItemIsSelectable|newItem->flags());
    m_ui->updateUrlList->insertItem(newItemIdx, newItem);
    m_ui->updateUrlList->editItem(m_ui->updateUrlList->item(newItemIdx));
}

void MainWindow::on_deleteListSource_clicked()
{
    int curr = m_ui->updateUrlList->currentRow();
    delete m_ui->updateUrlList->takeItem(curr);
}
