#ifndef UPDATEWIDGET_H
#define UPDATEWIDGET_H

#include <QtCore>
#include <QtGui>
#include <QtNetwork>

class UpdateWidget : public QTextBrowser {
    Q_OBJECT
public:
    UpdateWidget(QWidget *parent = 0);
    ~UpdateWidget();

/*
protected:
    void changeEvent(QEvent *e);
*/

public:

    void checkUpdates( QWidget *parent, QString insteadBinary, bool automatically );

private:
    QHttp *m_listServer;
    QProgressDialog *m_listLoadProgress;
    bool m_automatically;

    QString localInsteadVersion;
    QString localLauncherVersion;
    QString remoteInsteadVersion;
    QString remoteLauncherVersion;
    QString urlInstead;
    QString urlLauncher;

    void refreshUpdateList( QString insteadBinary, bool automatically );
    void parseUpdateList( QXmlStreamReader *xml );
    void generateUpdateMessage();
    QString detectInsteadVersion(QString insteadBinary);
    int compareVersions( QString ver1, QString ver2 );

private slots:
    void listServerDone( bool );

};

#endif // UPDATEWIDGET_H
