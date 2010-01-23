#ifndef UPDATEWINDOW_H
#define UPDATEWINDOW_H

#include <QtGui/QDialog>
#include <QHttp>
#include <QProgressDialog>
#include <QXmlStreamReader>
#include <QProcess>

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

    static void checkUpdates( QWidget *parent, QString insteadBinary, bool automatically );

private:
    Ui::UpdateWindow *m_ui;
    QHttp *m_listServer;
    QProgressDialog *m_listLoadProgress;
    bool m_automatically;
    QProcess *m_process;
    QString m_insteadBinary;

    QString localInsteadVersion;
    QString localLauncherVersion;
    QString remoteInsteadVersion;
    QString remoteLauncherVersion;
    QString urlInstead;
    QString urlLauncher;

    void refreshUpdateList( QString insteadBinary, bool automatically );
    void parseUpdateList( QXmlStreamReader *xml );
    void generateUpdateMessage();
    void detectInsteadVersion();

private slots:
    void listServerDone( bool );
    void processFinished( int exitCode, QProcess::ExitStatus exitStatus );
    void processStarted();
    void processError( QProcess::ProcessError );

};

#endif // UPDATEWINDOW_H
