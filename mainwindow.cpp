#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qunzip.h"
#include <QDebug>
#include <QMessageBox>
#include <QRegExp>
#include <QHttp>
#include <QProgressDialog>
#include <QDir>
#include <QTreeWidget>
#include <QXmlStreamReader>
#include <QTemporaryFile>


class NetGameItem: public QTreeWidgetItem {
    public:
	NetGameItem() {
	}
	
	NetGameItem( QTreeWidget *parent ) : QTreeWidgetItem( parent, QTreeWidgetItem::Type ) {
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
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    refreshLocalGameList();

    m_listServer = new QHttp(this);
    connect( m_listServer, SIGNAL( done( bool ) ), this, SLOT( listServerDone( bool ) ) );

    m_listLoadProgress = new QProgressDialog(parent);
    m_listLoadProgress->setLabelText("Загрузка списка игр ...");
    connect( m_listLoadProgress, SIGNAL(canceled()), m_listServer, SLOT(abort()));
    connect( m_listServer, SIGNAL( dataReadProgress( int, int ) ), m_listLoadProgress, SLOT( setValue( int ) ) );

    m_gameServer = new QHttp( this );
    connect( m_gameServer, SIGNAL( done(bool ) ), this, SLOT( gameServerDone( bool ) ) );
    connect( m_gameServer, SIGNAL( responseHeaderReceived( QHttpResponseHeader ) ), this, SLOT( gameServerResponseHeaderReceived( QHttpResponseHeader ) ) );

    m_gameLoadProgress = new QProgressDialog(parent);
    connect( m_gameServer, SIGNAL( dataReadProgress( int, int ) ), m_gameLoadProgress, SLOT( setValue( int ) ) );
    connect( m_gameLoadProgress, SIGNAL(canceled()), m_gameServer, SLOT(abort()));

    connect( ui->installPushButton, SIGNAL( clicked() ), this, SLOT( installPushButtonClicked() ) );
    connect( ui->refreshPushButton, SIGNAL( clicked() ), this, SLOT( refreshNetGameList() ) );
    connect( ui->playPushButton, SIGNAL( clicked() ), this, SLOT( playPushButtonClicked() ) );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::playPushButtonClicked()
{
    QString gameID = ui->listGames->currentItem()->data(0, Qt::UserRole).toString();

    // TODO запуск игры
    QMessageBox::information(this, "Запуск игры", gameID);
}

void MainWindow::refreshNetGameList()
{
    ui->listNewGames->clear();
    qDebug() << "Updating list from " << ui->lineUpdateUrl->text();
    QUrl url(ui->lineUpdateUrl->text());
    m_listServer->setHost(url.host());
    setEnabled( false );
    m_listServer->get(url.path());
//    m_listLoadProgress->setMinimumDuration(2000);
    m_listLoadProgress->setValue(0);
}

void MainWindow::installPushButtonClicked()
{
    ui->installPushButton->setDisabled(true);
    downloadGame(ui->listNewGames->currentItem());
}

void MainWindow::listServerDone(bool error)
{
    qDebug("List has been downloaded");

    setEnabled( true );
    m_listLoadProgress->reset();
    if(!error) {
        QXmlStreamReader xml( m_listServer->readAll() );
        while (!xml.atEnd()) {
            xml.readNext();
            if (xml.isStartElement() && xml.name() == "game_list" && xml.attributes().value("version") == "1.0") {
                parseGameList(&xml);
                break; // it should be only one game list
            }
        }
        if (xml.hasError()) {
            qWarning("Warning: errors while parsing game list");
            const QString s = xml.errorString();
            qWarning("%s", s.toLocal8Bit().data());
        }
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

void MainWindow::parseGameInfo( QXmlStreamReader *xml )
{
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
        }
        if( xml->isEndElement() && xml->name()=="game" )
            break;
    }
    // TODO: проверить что такой же версии игры нет в локальном списке
    if ( !hasLocalGame( info ) ) {
	qDebug("Adding game to the list %s", info.title().toLocal8Bit().data());
	NetGameItem *game = new NetGameItem( ui->listNewGames );
	game->setInfo( info );
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
    m_gameLoadProgress->setLabelText( QString( "Загрузка игры \"%1\"..." ).arg( ( ( NetGameItem *)game )->info().title() ) );
    m_gameLoadProgress->setValue(0);
}

void MainWindow::gameServerResponseHeaderReceived ( const QHttpResponseHeader & resp )
{
    qDebug("Header received. LEN=%d", resp.contentLength());
    m_gameLoadProgress->setMaximum(resp.contentLength());
}



void MainWindow::gameServerDone( bool error )
{
    setEnabled( true );
    m_gameLoadProgress->reset();
    ui->installPushButton->setEnabled(true);
    if(!error){
        const QString games_dir = getGameDirPath();
        const QString arch_name = games_dir + m_downloadingFileName;
        m_gameFile->copy( arch_name );
        qUnzip(arch_name, games_dir);
        QFile::remove( arch_name );
        QMessageBox::information(this, "Игра загружена и распакована", "Игра загружена и распакована");
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
    for( int i = 0; i < ui->listGames->topLevelItemCount(); i++ ) {
	LocalGameItem *item = ( ( LocalGameItem * )ui->listGames->topLevelItem( i ) );
	if ( item->info() == info )
	    return true;
    }
    return false;
}

// Обновление списка установленных игр
void MainWindow::refreshLocalGameList() {

    // очищаем список
    ui->listGames->clear();

    // получаем директорию с играми
    QString gamePath = getGameDirPath();
    QDir gameDir(gamePath);
    qDebug() << "game path: " << gamePath;

    // если директория не существует, то создаем ее
    if ( !gameDir.exists() ) {
        qDebug() << "created games directory";
        if ( !QDir::home().mkpath( ".instead/games" ) ) {
            QMessageBox::critical(this, "Ошибка", "Не удается создать каталог " + gamePath);
            return;
        }
    }

    // просматриваем все подкаталоги
    QStringList gameList = gameDir.entryList( QDir::AllDirs|QDir::NoDotAndDotDot, QDir::NoSort );
    QString gameID;
    foreach( gameID, gameList ) {
        GameInfo info;
        if ( getLocalGameInfo( gameDir, gameID, info ) ) {
            // TODO добавляем игру в список
            qDebug() << "found game " << info.name() << info.title() << info.version();
            LocalGameItem *game = new LocalGameItem( ui->listGames );
            game->setInfo( info );
        }
    }
    
//    -- $Name:Зеркало$
//-- $Version: 0.4.1$
}

QString MainWindow::getGameDirPath() const
{
#ifdef Q_OS_UNIX
    return QDir::home().absolutePath() + "/.instead/games/";

#elif Q_OS_WIN
#error "Please, provide a correct path to games folder in Windows OS"
    return ""; //TODO

#else
#error "Unsupported OS"
#endif
}
