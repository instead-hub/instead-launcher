#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QRegExp>
#include <QHttp>
#include <QProgressDialog>
#include <QDir>
#include <QTreeWidget>
#include <QXmlStreamReader>
#include <QTemporaryFile>

class GameItem: public QTreeWidgetItem
{
    public:
        void setVersion(const QString &ver){
            m_version=ver;
            setText(1, ver);
        }
        void setName(const QString &nm){
            m_name=nm;
            setText(0, nm);
        }
        void setUrl(const QString &url) {
            m_url = url;
        }

	QString name() {
	    return m_name;
	}
	
        const QString getVersion() {
            return m_version;
        }
        const QString getUrl() {
            return m_url;
        }
    private:
        QString m_version;
        QString m_url;
        QString m_name;
};


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    refreshLocalGameList();

    list_server = new QHttp(this);
    connect(list_server, SIGNAL(done(bool)), this, SLOT(on_list_server_done(bool)));
    connect( list_server, SIGNAL( readyRead( const QHttpResponseHeader & ) ), this, SLOT( readyRead( const QHttpResponseHeader & ) ) );

    m_listLoadProgress = new QProgressDialog(parent);
    m_listLoadProgress->setLabelText("Загрузка списка игр ...");
    m_listLoadProgress->setWindowModality(Qt::WindowModal);
    connect(m_listLoadProgress, SIGNAL(canceled()), list_server, SLOT(abort()));
    connect( list_server, SIGNAL( dataReadProgress( int, int ) ), m_listLoadProgress, SLOT( setValue( int ) ) );

    game_server = new QHttp(this);
    connect(game_server, SIGNAL(done(bool)), this, SLOT(on_game_server_done(bool)));
    connect(game_server, SIGNAL(responseHeaderReceived(QHttpResponseHeader)), this, SLOT(on_game_server_responseHeaderReceived(QHttpResponseHeader)));

    m_gameLoadProgress = new QProgressDialog(this);
//    m_gameLoadProgress->setLabelText("Загрузка игры GAME_NAME...");
    m_gameLoadProgress->setWindowModality(Qt::WindowModal);
    connect( game_server, SIGNAL( dataReadProgress( int, int ) ), m_gameLoadProgress, SLOT( setValue( int ) ) );
    connect(m_gameLoadProgress, SIGNAL(canceled()), game_server, SLOT(abort()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_buttonPlay_clicked()
{
    QString gameID = ui->listGames->currentItem()->data(0, Qt::UserRole).toString();

    // TODO запуск игры
    QMessageBox::information(this, "Запуск игры", gameID);
}

void MainWindow::on_buttonRefresh_clicked()
{
    ui->listNewGames->clear();
    qDebug() << "Updating list from " << ui->lineUpdateUrl->text();
    QUrl url(ui->lineUpdateUrl->text());
    list_server->setHost(url.host());

    list_server->get(url.path());
//    m_listLoadProgress->setMinimumDuration(2000);
    m_listLoadProgress->setValue(0);
}

void MainWindow::on_list_server_done(bool error)
{
    qDebug("List has been downloaded");

    m_listLoadProgress->reset();
    if(!error) {
        QXmlStreamReader xml(list_server->readAll());
        while (!xml.atEnd()) {
            xml.readNext();
            if (xml.isStartElement() && xml.name() == "game_list" && xml.attributes().value("version") == "1.0") {
                readGameList(&xml);
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

void MainWindow::readGameList(QXmlStreamReader *xml)
{
    Q_ASSERT(xml->name() == "game_list");
    while (!xml->atEnd()) {
        xml->readNext();
        if (xml->isStartElement() && xml->name() == "game") {
            qDebug("A game in the list!");
            readGame(xml);
        }
    }
}

void MainWindow::readGame(QXmlStreamReader *xml)
{
    Q_ASSERT(xml->name() == "game");
    GameItem *game = new GameItem();
    while (!xml->atEnd()) {
        xml->readNext();
        if (xml->isStartElement()) {
            if(xml->name() == "name") {
                const QString s = xml->readElementText();
                game->setName(s);
            }
            else if(xml->name() == "version") {
                const QString s = xml->readElementText();
                game->setVersion(s);
            }
            else if(xml->name() == "url") {
                const QString s = xml->readElementText();
                game->setUrl(s);
            }
        }
        if(xml->isEndElement() && xml->name()=="game")
        {
            break;
        }
    }
    // TODO: проверить что такой же версии игры нет в локальном списке
    qDebug("Adding game to the list %s", game->getVersion().toLocal8Bit().data());
    ui->listNewGames->addTopLevelItem(game);
}


void MainWindow::on_buttonInstall_clicked()
{
    ui->buttonInstall->setDisabled(true);
    downloadGame(ui->listNewGames->currentItem());
}

void MainWindow::downloadGame(QTreeWidgetItem *game)
{
    Q_ASSERT(game!=NULL);
    m_gameFile = new QTemporaryFile();
    QUrl url(static_cast<GameItem*>(game)->getUrl());
    game_server->setHost(url.host());
    game_server->get(url.path(), m_gameFile);
    m_downloadingFileName = url.path().split( "/" ).last();
    m_gameLoadProgress->setLabelText( QString( "Загрузка игры \"%1\"..." ).arg( ( ( GameItem * )game )->name() ) );
    m_gameLoadProgress->setValue(0);
}

void MainWindow::on_game_server_responseHeaderReceived ( const QHttpResponseHeader & resp )
{
    qDebug("Header received. LEN=%d", resp.contentLength());
    m_gameLoadProgress->setMaximum(resp.contentLength());
}

void MainWindow::on_game_server_done(bool error)
{
    m_gameLoadProgress->reset();
    if(!error){
        ui->buttonInstall->setEnabled(true);
        m_gameFile->copy(QDir::home().absolutePath()+"/.instead/games/" + m_downloadingFileName);
        QMessageBox::information(this, "Игра загружена", "Игра загружена");
    }
    else {
        ui->buttonInstall->setEnabled(true);
        qWarning("WARN: Game load error");
    }
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

    info.id = gameID;
    info.name = name;
    info.version = version;

    return true;
}

// Обновление списка установленных игр
void MainWindow::refreshLocalGameList() {

    // очищаем список
    ui->listGames->clear();

    // получаем директорию с играми
    QString gamePath = QDir::home().absolutePath() + "/.instead/games";
    QDir gameDir(gamePath);
    qDebug() << "Game path" << gamePath;

    // если директория не существует, то создаем ее
    if (!gameDir.exists()) {
        qDebug() << "Created games directory";
        if (!QDir::home().mkpath(".instead/games")) {
            QMessageBox::critical(this, "Ошибка", "Не удается создать каталог "+gamePath);
            return;
        }
    }

    // просматриваем все подкаталоги
    QStringList gameList = gameDir.entryList(QDir::AllDirs|QDir::NoDotAndDotDot, QDir::NoSort);
    QString gameID;
    foreach (gameID, gameList) {
        GameInfo info;
        if (getLocalGameInfo(gameDir, gameID, info)) {
            // TODO добавляем игру в список
            qDebug() << "Found game " << info.id << info.name << info.version;
            QTreeWidgetItem * item = new QTreeWidgetItem(ui->listGames);
            item->setText(0, info.name);
            item->setText(1, info.version);
            item->setData(0, Qt::UserRole, info.id);
        }
    }
    
//    -- $Name:Зеркало$
//-- $Version: 0.4.1$
}
