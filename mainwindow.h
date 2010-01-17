#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtCore/QTimer>

class QDir;
class QHttp;
class QXmlStreamReader;
class QProgressDialog;
class QTreeWidgetItem;
class QTemporaryFile;
class QHttpResponseHeader;
class QTreeWidgetItem;

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
    Ui::MainWindow *ui;

    QHttp *list_server;
    QProgressDialog *m_listLoadProgress;
    void readGameList(QXmlStreamReader *xml);
    void readGame(QXmlStreamReader *xml);

    QHttp *game_server;
    QProgressDialog *m_gameLoadProgress;
    QTemporaryFile *m_gameFile;
    void downloadGame(QTreeWidgetItem *game);

    QString m_downloadingFileName;

    void refreshLocalGameList();
    bool getLocalGameInfo(const QDir gameDir, const QString gameID, GameInfo &info);

private slots:
    void on_buttonInstall_clicked();
    void on_buttonRefresh_clicked();
    void on_buttonPlay_clicked();

    void on_list_server_done(bool error);
    void on_game_server_responseHeaderReceived ( const QHttpResponseHeader & resp );
    void on_game_server_done(bool error);
};

#endif // MAINWINDOW_H
