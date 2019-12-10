#include "server.h"

#include <QWebSocket>
#include <QWebSocketServer>

Server::Server(const QString &name, QObject *parent) :
    QObject(parent)
{
    mSocketServer.reset(new QWebSocketServer(name, QWebSocketServer::NonSecureMode));
    mSocketServer->setProxy(QNetworkProxy::NoProxy);

    connect(mSocketServer.data(), &QWebSocketServer::newConnection, this, &Server::onNewConnection);
}

void Server::listen(quint16 port)
{
    bool value = mSocketServer->listen(QHostAddress::AnyIPv4, port);
    emit listening(value);
}

void Server::onNewConnection()
{
    QWebSocket *_socket = mSocketServer->nextPendingConnection();
    QWebSocketPtr clientSocket(_socket);
    mClientHash[_socket] = clientSocket;
    emit connected(clientSocket);

    connect(clientSocket.data(), &QWebSocket::disconnected, this, &Server::onDisconnected);
    connect(clientSocket.data(), &QWebSocket::textMessageReceived, this, &Server::onTextMessageReceived);
    connect(clientSocket.data(), &QWebSocket::binaryMessageReceived, this, &Server::onBinaryMessageReceived);
}

void Server::onDisconnected()
{
    QWebSocket *socket = static_cast<QWebSocket *>(QObject::sender());
    QWebSocketPtr client(mClientHash[socket]);
    mClientHash.remove(socket);
    emit disconnected(client);
}

void Server::onTextMessageReceived(const QString &text)
{
    QWebSocket *socket = static_cast<QWebSocket *>(QObject::sender());
    emit textMessageReceived(mClientHash[socket], text);
}

void Server::onBinaryMessageReceived(const QByteArray &data)
{
    QWebSocket *socket = static_cast<QWebSocket *>(QObject::sender());
    emit binaryMessageReceived(mClientHash[socket], data);
}

QList<QWebSocketPtr> Server::clientList()
{
    QList<QWebSocketPtr> resultList;
    QHashIterator<QWebSocket *, QWebSocketPtr> iterator(mClientHash);
    while (iterator.hasNext()) {
        iterator.next();
        resultList << iterator.value();
    }

    return resultList;
}
