#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <networkmanager.h>
#include <QHostAddress>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    NetworkManager network;

private slots:
    void on_PBnewConnection_clicked();

    void on_CBInterFaces_currentIndexChanged(int index);

    void on_PBSent_clicked();

    void handlerMessageReceived(QHostAddress ip, quint16 port, QByteArray message);

    void on_LEMessage_returnPressed();

private:
    QListWidgetItem *createItem(QHostAddress ip, quint16 port);
    void sentMessage();
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
