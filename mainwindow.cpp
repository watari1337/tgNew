#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialogip.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , network(this)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QList<QString> ifaces = network.getNameInterFace();
    for(int i = 0; i < ifaces.size(); i++){
        ui->CBInterFaces->addItem(ifaces[i]);
        if(ifaces[i] == "Беспроводная сеть"){
            network.getIp(i);
            ui->CBInterFaces->setCurrentIndex(i);
        }
    }

    ui->LMyAddr->setText(QString("ip: %1, port %2").arg(network.getMyIp().toString()).arg(network.getMyPort()));
    connect(&network, &NetworkManager::messageReceived, this, &MainWindow::handlerMessageReceived);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_PBnewConnection_clicked()
{
    DialogIP dialog(this);

    if(dialog.exec() == QDialog::Accepted){
        createItem(dialog.ip, dialog.port);
    }
}


void MainWindow::on_CBInterFaces_currentIndexChanged(int index)
{
    network.getIp(index);
    ui->LMyAddr->setText(QString("ip: %1, port %2").arg(network.getMyIp().toString()).arg(network.getMyPort()));
}


void MainWindow::on_PBSent_clicked()
{
    sentMessage();
}

void MainWindow::handlerMessageReceived(QHostAddress ip, quint16 port, QByteArray message)
{
    QListWidgetItem* findItem = NULL;
    for (int i = 0; i < ui->LWNames->count(); ++i) {
        QListWidgetItem* item = ui->LWNames->item(i);
        if (/*item->data(Qt::UserRole + 1).toUInt() == port &&*/
            QHostAddress(item->data(Qt::UserRole).toString()) == ip) {
            findItem = item;
            ui->LWNames->setCurrentItem(findItem);
        }
    }
    if (!findItem) {
        createItem(ip, port);
    }
    QString text = ui->PTEMessage->toPlainText() + QString::fromUtf8(message);
    ui->PTEMessage->setPlainText(text);
}

QListWidgetItem *MainWindow::createItem(QHostAddress ip, quint16 port)
{
    QListWidgetItem *newItem = new QListWidgetItem(QString("ip: %1, port %2").arg(ip.toString()).arg(port));
    newItem->setData(Qt::UserRole, ip.toString());
    newItem->setData(Qt::UserRole + 1, port);
    ui->LWNames->addItem(newItem);
    ui->LWNames->setCurrentItem(newItem);
    return newItem;
}

void MainWindow::sentMessage()
{
    if (!ui->LWNames->currentItem()) return;
    QHostAddress ip(ui->LWNames->currentItem()->data(Qt::UserRole).toString());
    quint16 port = ui->LWNames->currentItem()->data(Qt::UserRole + 1).toUInt();
    QString text = ui->LEMessage->text();
    if (text.size() > 0){
        ui->LEMessage->clear();
        QString textToSent = text + "\n";
        text = ui->PTEMessage->toPlainText() + textToSent;
        ui->PTEMessage->setPlainText(text);
        QByteArray message = textToSent.toUtf8();
        network.sentMessage(ip, port, message);
    }
}

void MainWindow::on_LEMessage_returnPressed()
{
    sentMessage();
}

