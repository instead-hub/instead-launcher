#ifndef ABOUTWIDGET_H
#define ABOUTWIDGET_H

#include <QtGui>
#include <QtCore>

class AboutWidget : public QWidget
{
    Q_OBJECT

public:

    AboutWidget(QWidget *parent = 0);
    ~AboutWidget();

signals:

    void checkUpdatesRequest();

private slots:

    void linkActivated( const QString &link );
    void goToDevelopersForum();
    void goToINSTEADPage();
    void goToINSTEADLauncherPage();

private:

    QLabel *m_logoLabel;

};

#endif // ABOUTWIDGET_H
