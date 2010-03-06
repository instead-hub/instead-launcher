#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <QtCore>
#include <QtNetwork>

namespace Ui
{
    class MainWindow;
}

class UpdateWidget;

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

    virtual ~GameInfo() {
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
	m_depends = info.m_depends;
    }

    virtual NetGameInfo operator=( const NetGameInfo &info ) {
	GameInfo::operator=( info );
	m_url = info.m_url;
	m_md5 = info.m_md5;
	m_instead = info.m_instead;
	m_descUrl = info.m_descUrl;
	m_depends = info.m_depends;
	return *this;
    }

    virtual ~NetGameInfo() {
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

    void setDepends( const QStringList &depends ) {
	m_depends = depends;
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

    QStringList depends() {
	return m_depends;
    }

    private:
	QString m_url;
	QString m_md5;
	QString m_instead;
	QString m_descUrl;
	QStringList m_depends;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:

    QList<QTreeWidgetItem *> findEssentialGames( QTreeWidgetItem * );

    bool getLocalGameInfo( const QDir gameDir, const QString gameID, GameInfo &info );
    bool hasLocalGame( const GameInfo & );

    void parseGameList( QXmlStreamReader *xml );
    void parseGameInfo( QXmlStreamReader *xml );

    bool downloadGame(QTreeWidgetItem *game);
//    QString getGameDirPath() const;
//    QString getDefaultInterpreterPath() const;
//    QString getConfigPath() const;
//    bool checkOrCreateGameDir(QString gameDir);

    void resetConfig();
    void loadConfig();
    void saveConfig();

    QNetworkProxy *m_networkProxy;
    UpdateWidget *m_swUpdateWidget;
    QTemporaryFile *m_gameFile;
    QHttp *m_listServer, *m_gameServer;
    QProgressDialog *m_listLoadProgress, *m_gameLoadProgress;
    QString m_downloadingFileName;
    QProcess *m_process;
    Ui::MainWindow *m_ui;
    bool listIsDirty;

private slots:

    void refreshLocalGameList();
    void refreshNetGameList(bool next=false);

    void installSelectedGame();

    void resetPushButtonClicked();
    void detailsLinkClicked( const QString & );
    void playSelectedGame();
    void browseInsteadPath();
    void browseGamesPath();
    void removeSelectedGame();

    void listServerDone( bool );
    void gameServerResponseHeaderReceived( const QHttpResponseHeader & );
    void gameServerDone( bool error );

    void processFinished( int exitCode, QProcess::ExitStatus exitStatus );
    void processStarted();
    void processError( QProcess::ProcessError );

    void addSourcePushButtonClicked();
    void deleteSourcePushButtonClicked();

    void gamesDirChanged();
    void tabChanged(int pos);
    void updateProxy();

    void checkUpdates();

};

#endif // MAINWINDOW_H
