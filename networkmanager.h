#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QNetworkInterface>
#include <QVector>
#include <QAbstractSocket>

class NetworkManager: public QObject
{
    Q_OBJECT

    struct clientWeb{
        clientWeb(QHostAddress ip, quint16 port): m_ip(ip), m_port(port) {
            m_socket = new QTcpSocket();
        }
        clientWeb(QTcpSocket *socket): m_socket(socket) {
            m_ip = socket->peerAddress();
            m_port = socket->peerPort();
        }

        bool operator==(const clientWeb &other){
            return m_ip == other.m_ip && m_port == other.m_port;
        }

        QHostAddress m_ip;
        quint16 m_port;
        QTcpSocket *m_socket;
        QVector<QByteArray> dataToSent;
    };

public:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();
    bool startServer(int port = 0); //0 - выбор свободного порта
    void stopServer();
    void sentMessage(QHostAddress ip, quint16 port, QByteArray message);
    QList<QString> getNameInterFace();
    QHostAddress getIp(int nInterface);
    QHostAddress getMyIp();
    quint16 getMyPort();

private:
    //bool connectToHost(QHostAddress ip, quint16 port);
    clientWeb *findOrCreateClient(QHostAddress ip, quint16 port);
    clientWeb *findOrCreateClient(QTcpSocket *socket);
    QTcpServer *server;
    QList<clientWeb*> clients;
    QList<QNetworkInterface> interfaces;
    QNetworkInterface interface;
    QHostAddress m_myIp;
    quint16 m_myPort;

signals:
    void messageReceived(QHostAddress ip, quint16 port, QByteArray message);

private slots:
    void handleNewConnection();

    void handleReadMessage();
    void handleDisconnectSocket();
    void handleConnectSocketToPear();
    void handleOnSocketChanged(QAbstractSocket::SocketState state);

};

#endif // NETWORKMANAGER_H
