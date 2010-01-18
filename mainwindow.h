#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <QtCore>
#include <QtXml>
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
    
    QString name() const {
	return m_name;
    }
    
    QString title() const {
	return m_title;
    }
    
    QString version() const {
	return m_version;
    }
    
    virtual GameInfo operator=( const GameInfo &info ) {
	m_name = info.m_name;
	m_title = info.m_title;
	m_version = info.m_version;
	
	return *this;
    }

    bool operator==( const GameInfo &info ) {
	return ( m_name == info.m_name ) && ( m_version == info.m_version );
    }

private:
    QString m_name;
    QString m_title;
    QString m_version;
};

class NetGameInfo : public GameInfo {

public:
    NetGameInfo() {
    }

    NetGameInfo( const NetGameInfo &info ) : GameInfo( info ) {
        m_url = info.m_url;
    }

    virtual NetGameInfo operator=( const NetGameInfo &info ) {
	GameInfo::operator=( info );
	m_url = info.m_url;
	
	return *this;
    }

    ~NetGameInfo() {
    }

    void setUrl( const QString &url ) {
	m_url = url;
    }

    QString url() {
	return m_url;
    }

    private:
	QString m_url;
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

    QTemporaryFile *m_gameFile;
    QHttp *m_listServer, *m_gameServer;
    QProgressDialog *m_listLoadProgress, *m_gameLoadProgress;
    QString m_downloadingFileName;
    QProcess *m_process;
    Ui::MainWindow *ui;

private slots:

    void refreshLocalGameList();
    void refreshNetGameList();

    void installPushButtonClicked();

    void playPushButtonClicked();

    void listServerDone( bool );
    void gameServerResponseHeaderReceived( const QHttpResponseHeader & );
    void gameServerDone( bool error );

    void processFinished( int exitCode, QProcess::ExitStatus exitStatus );
};

#endif // MAINWINDOW_H
