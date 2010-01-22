#ifndef UPDATEWINDOW_H
#define UPDATEWINDOW_H

#include <QtGui/QDialog>
#include <QHttp>
#include <QProgressDialog>
#include <QXmlStreamReader>

namespace Ui {
    class UpdateWindow;
}

class UpdateWindow : public QDialog {
    Q_OBJECT
public:
    UpdateWindow(QWidget *parent = 0);
    ~UpdateWindow();

protected:
    void changeEvent(QEvent *e);

public:

    static void checkUpdates( QWidget *parent, bool automatically );

private:
    Ui::UpdateWindow *m_ui;
    QHttp *m_listServer;
    QProgressDialog *m_listLoadProgress;
    bool m_automatically;

    QString localInsteadVersion;
    QString localLauncherVersion;
    QString remoteInsteadVersion;
    QString remoteLauncherVersion;
    QString urlInstead;
    QString urlLauncher;

    void refreshUpdateList( bool automatically );
    void parseUpdateList( QXmlStreamReader *xml );
    void refreshLocalVersions();
    void generateUpdateMessage();

private slots:
    void listServerDone( bool );

};

#endif // UPDATEWINDOW_H
