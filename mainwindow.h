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

struct GameInfo {
    QString id;         // cat
    QString name;       // Возвращение квантового кота
    QString version;    // 0.3.1
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    bool getLocalGameInfo( const QDir gameDir, const QString gameID, GameInfo &info );

    void parseGameList( QXmlStreamReader *xml );
    void parseGameInfo( QXmlStreamReader *xml );

    void downloadGame(QTreeWidgetItem *game);
    QString getGameDirPath() const;

    QTemporaryFile *m_gameFile;
    QHttp *m_listServer, *m_gameServer;
    QProgressDialog *m_listLoadProgress, *m_gameLoadProgress;
    QString m_downloadingFileName;
    Ui::MainWindow *ui;

private slots:

    void refreshLocalGameList();

    void installPushButtonClicked();
    void refreshPushButtonClicked();
    void playPushButtonClicked();

    void listServerDone( bool );
    void gameServerResponseHeaderReceived( const QHttpResponseHeader & );
    void gameServerDone( bool error );
};

#endif // MAINWINDOW_H
