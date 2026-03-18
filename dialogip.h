#ifndef DIALOGIP_H
#define DIALOGIP_H

#include <QDialog>
#include "mainwindow.h"
#include <QHostAddress>

namespace Ui {
class DialogIP;
}

class DialogIP : public QDialog
{
    Q_OBJECT

public:
    explicit DialogIP(MainWindow *parent = nullptr);
    ~DialogIP();
    QHostAddress ip;
    quint16 port;

private:
    Ui::DialogIP *ui;
    MainWindow *mainWindow;

protected:
    void accept() override;
};

#endif // DIALOGIP_H
