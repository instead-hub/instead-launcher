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

private:
    Ui::UpdateWindow *m_ui;
    QHttp *m_listServer;
    QProgressDialog *m_listLoadProgress;

    QString localInsteadVersion;
    QString localLauncherVersion;
    QString remoteInsteadVersion;
    QString remoteLauncherVersion;
    QString urlInstead;
    QString urlLauncher;

    void refreshUpdateList();
    void parseUpdateList( QXmlStreamReader *xml );

private slots:
    void listServerDone( bool );

};

#endif // UPDATEWINDOW_H
