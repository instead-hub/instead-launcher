#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <QtCore>
#include <QtNetwork>

namespace Ui
{
    class MainWindow;
}

class GameInfo {

public:
    GameInfo() {
    }

    GameInfo( const GameInfo &info ) {
	m_name = info.m_name;
	m_title = info.m_title;
	m_version = info.m_version;
	m_lang = info.m_lang;
    }

    ~GameInfo() {
    }

    void setName( const QString &name ) {        
	m_name = name;
    }

    void setTitle( const QString &title ) {
	m_title = title;
    }

    void setVersion( const QString &version ) {
	m_version = version;
    }

    void setLang( const QString &lang ) {
	m_lang = lang;
    }

    QString name() const {
	return m_name;
    }

    QString title() const {
	return m_title;
    }

    QString version() const {
	return m_version;
    }

    QString lang() const {
	return m_lang;
    }

    virtual GameInfo operator=( const GameInfo &info ) {
	m_name = info.m_name;
	m_title = info.m_title;
	m_version = info.m_version;
	m_lang = info.m_lang;

	return *this;
    }

    bool operator==( const GameInfo &info ) {
	return ( m_name == info.m_name ) && ( m_version == info.m_version ); // && ( m_lang == info.m_lang );
    }

private:
    QString m_name;
    QString m_title;
    QString m_version;
    QString m_lang;
};

class NetGameInfo : public GameInfo {

public:
    NetGameInfo() {
    }

    NetGameInfo( const NetGameInfo &info ) : GameInfo( info ) {
        m_url = info.m_url;
	m_md5 = info.m_md5;
	m_instead = info.m_instead;
	m_descUrl = info.m_descUrl;
    }

    virtual NetGameInfo operator=( const NetGameInfo &info ) {
	GameInfo::operator=( info );
	m_url = info.m_url;
	m_md5 = info.m_md5;
	m_instead = info.m_instead;
	m_descUrl = info.m_descUrl;
	return *this;
    }

    ~NetGameInfo() {
    }

    void setUrl( const QString &url ) {
	m_url = url;
    }

    void setMD5( const QString &md5 ) {
	m_md5 = md5;
    }

    void setInstead( const QString &instead ) {
	m_instead = instead;
    }

    void setDescUrl( const QString &descUrl ) {
	m_descUrl = descUrl;
    }

    QString url() {
	return m_url;
    }
    
    QString MD5() {
	return m_md5;
    }

    QString instead() {
	return m_instead;
    }

    QString descUrl() {
	return m_descUrl;
    }

    private:
	QString m_url;
	QString m_md5;
	QString m_instead;
	QString m_descUrl;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:

    bool getLocalGameInfo( const QDir gameDir, const QString gameID, GameInfo &info );
    bool hasLocalGame( const GameInfo & );

    void parseGameList( QXmlStreamReader *xml );
    void parseGameInfo( QXmlStreamReader *xml );

    void downloadGame(QTreeWidgetItem *game);
    QString getGameDirPath() const;
    QString getDefaultInterpreterPath() const;
    QString getConfigPath() const;
    bool checkOrCreateGameDir();

    void resetConfig();
    void loadConfig();
    void saveConfig();

    QTemporaryFile *m_gameFile;
    QHttp *m_listServer, *m_gameServer;
    QProgressDialog *m_listLoadProgress, *m_gameLoadProgress;
    QString m_downloadingFileName;
    QProcess *m_process;
    Ui::MainWindow *m_ui;

private slots:

    void refreshLocalGameList();
    void refreshNetGameList(bool next=false);

    void installPushButtonClicked();

    void resetPushButtonClicked();
    void playPushButtonClicked();
    void detailsPushButtonClicked();

    void listServerDone( bool );
    void gameServerResponseHeaderReceived( const QHttpResponseHeader & );
    void gameServerDone( bool error );

    void processFinished( int exitCode, QProcess::ExitStatus exitStatus );
    void processStarted();
    void processError( QProcess::ProcessError );

    void on_addListSource_clicked();
    void on_deleteListSource_clicked();
};

#endif // MAINWINDOW_H
