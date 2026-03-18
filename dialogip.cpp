#include "dialogip.h"
#include "ui_dialogip.h"
#include <QDebug>

DialogIP::DialogIP(MainWindow *parent)
    : QDialog(parent)
    , ui(new Ui::DialogIP)
{
    mainWindow = parent;
    ui->setupUi(this);
    //ui->LIpPort->setText(mainWindow->network.getMyIp());
}

DialogIP::~DialogIP()
{
    delete ui;
}

void DialogIP::accept()
{
    QHostAddress addr;
    if (!addr.setAddress(ui->LEIp->text())) return;
    ip = addr;
    port = ui->SPPort->value();
    QDialog::accept();
}
