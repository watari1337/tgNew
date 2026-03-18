#include "networkmanager.h"
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent):
    QObject(parent),
    server(new QTcpServer(this))
{
    connect(server, &QTcpServer::newConnection, this, &NetworkManager::handleNewConnection);
    interfaces = QNetworkInterface::allInterfaces();
    startServer();
}

NetworkManager::~NetworkManager()
{
    stopServer();
}

bool NetworkManager::startServer(int port)
{
    if (!server->listen(QHostAddress::AnyIPv4, port)){
        qDebug() << "порт занят!";
        return false;
    } else {
        qDebug() << "connection open";
        m_myPort = server->serverPort();
        return true;
    }
}

void NetworkManager::stopServer()
{
    for (int i = 0; i < clients.size(); i++){
        clients[i]->m_socket->disconnectFromHost();
        clients[i]->m_socket->deleteLater();
    }
    clients.clear();
    server->close();
}

QList<QString> NetworkManager::getNameInterFace()
{
    QList<QString> answer;
    for (int i = 0; i < interfaces.size(); ++i) {
        answer.push_back(interfaces[i].humanReadableName());
    }
    return answer;
}

QHostAddress NetworkManager::getIp(int nInterface)
{
    if ((nInterface < 0) || (nInterface >= interfaces.size())){
        qDebug() << "неверно передан параметр ip";
        return QHostAddress();
    }
    auto entries = interfaces[nInterface].addressEntries();
    for (auto entry: entries){
        if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol){
            m_myIp = entry.ip();
            return entry.ip();
        }
    }
    return QHostAddress();
}

QHostAddress NetworkManager::getMyIp()
{
    return m_myIp;
}

quint16 NetworkManager::getMyPort()
{
    return m_myPort;
}

void NetworkManager::handleNewConnection()
{
    qDebug() << "new connection!";
    while (server->hasPendingConnections()) {
        QTcpSocket *clientSocket = server->nextPendingConnection();
        findOrCreateClient(clientSocket);
    }
}

void NetworkManager::sentMessage(QHostAddress ip, quint16 port, QByteArray message)
{
    clientWeb *client = findOrCreateClient(ip, port);
    client->dataToSent.push_back(message);
    if (client->m_socket->state() == QAbstractSocket::ConnectedState){
        for (int i = client->dataToSent.size()-1; i >= 0; i--) {
            qDebug() << "try write";
            client->m_socket->write(client->dataToSent[i]);
            client->dataToSent.remove(i);
        }
    } if (client->m_socket->state() == QAbstractSocket::UnconnectedState){
        qDebug() << "connecting to" << ip << port;
        client->m_socket->connectToHost(ip, port);
    }
}

NetworkManager::clientWeb *NetworkManager::findOrCreateClient(QHostAddress ip, quint16 port)
{
    int index = -1;
    for (int i = 0; i < clients.size(); ++i) {
        if (clients[i]->m_ip == ip && clients[i]->m_port == port){
            index = i;
        }
    }
    if (index == -1) {
        clientWeb *client = new clientWeb(ip, port);
        clients.push_back(client);
        index = clients.size()-1;
        connect(clients[index]->m_socket, &QTcpSocket::connected, this, &NetworkManager::handleConnectSocketToPear);
        connect(clients[index]->m_socket, &QTcpSocket::readyRead, this, &NetworkManager::handleReadMessage);
        connect(clients[index]->m_socket, &QTcpSocket::disconnected, this, &NetworkManager::handleDisconnectSocket);
        connect(clients[index]->m_socket, &QTcpSocket::stateChanged, this, &NetworkManager::handleOnSocketChanged);
    }
    return clients[index];
}

NetworkManager::clientWeb *NetworkManager::findOrCreateClient(QTcpSocket *socket)
{
    int index = -1;
    for (int i = 0; i < clients.size(); ++i) {
        if (clients[i]->m_ip == socket->peerAddress() && clients[i]->m_port == socket->peerPort()){
            index = i;
            if (clients[i]->m_socket == socket) return clients[i];
        }
    }
    connect(socket, &QTcpSocket::connected, this, &NetworkManager::handleConnectSocketToPear);
    connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::handleReadMessage);
    connect(socket, &QTcpSocket::disconnected, this, &NetworkManager::handleDisconnectSocket);
    connect(socket, &QTcpSocket::stateChanged, this, &NetworkManager::handleOnSocketChanged);
    if (index == -1) {
        clientWeb *client = new clientWeb(socket);
        clients.push_back(client);
        index = clients.size()-1;
    } else {
        clients[index]->m_socket->deleteLater();
        clients[index]->m_socket = socket;
    }
    return clients[index];
}

void NetworkManager::handleReadMessage()
{
    qDebug() << "start handleRead";
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;
    QByteArray data = clientSocket->readAll();
    emit messageReceived(clientSocket->peerAddress(), clientSocket->peerPort(), data);
    QString text = QString::fromUtf8(data);

    qDebug() << text;
}

void NetworkManager::handleDisconnectSocket()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    clientWeb *client = findOrCreateClient(clientSocket->peerAddress(), clientSocket->peerPort());
    if (client->dataToSent.size()) {
        client->m_socket->connectToHost(client->m_ip, client->m_port);
    }
}

void NetworkManager::handleConnectSocketToPear()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    clientWeb *client = findOrCreateClient(clientSocket->peerAddress(), clientSocket->peerPort());
    for (int i = client->dataToSent.size()-1; i >= 0; i--) {
        qDebug() << "connected and try write";
        client->m_socket->write(client->dataToSent[i]);
        client->dataToSent.remove(i);
    }
}

void NetworkManager::handleOnSocketChanged(QAbstractSocket::SocketState state)
{
    auto *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    qDebug() << "state:" << state
             << "local:" << socket->localAddress() << socket->localPort()
             << "peer:"  << socket->peerAddress()  << socket->peerPort();
}
