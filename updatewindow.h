#ifndef UPDATEWINDOW_H
#define UPDATEWINDOW_H

#include <QtGui/QDialog>

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
};

#endif // UPDATEWINDOW_H
