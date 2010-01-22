#include "updatewindow.h"
#include "ui_updatewindow.h"

UpdateWindow::UpdateWindow(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::UpdateWindow)
{
    m_ui->setupUi(this);
}

UpdateWindow::~UpdateWindow()
{
    delete m_ui;
}

void UpdateWindow::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
