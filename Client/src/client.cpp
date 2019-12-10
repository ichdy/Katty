#include "client.h"

#include <QCoreApplication>
#include <QWebSocket>
#include <QThread>
#include <QDebug>

Client::Client(const QString &name, QObject *parent) :
    QObject(parent),
    mName(name)
{
    mSocket.reset(new QWebSocket);

    connect(mSocket.data(), &QWebSocket::connected, this, &Client::connected);
    connect(mSocket.data(), &QWebSocket::disconnected, this, &Client::disconnected);
    connect(mSocket.data(), &QWebSocket::binaryMessageReceived, this, &Client::binaryMessageReceived);
}

void Client::open(const QString &host, quint16 port)
{
    mHost = host;
    mPort = port;

    QUrl url;
    url.setScheme("ws");
    url.setHost(mHost);
    url.setPort(mPort);

    mSocket->open(url);
}

void Client::close()
{
    if (isConnected())
        mSocket->close();
    else
        mSocket->abort();
}

void Client::send(const QByteArray &data)
{
    if (!isConnected())
        return;

    mSocket->sendBinaryMessage(data);
}

void Client::waitForConnected()
{
    if (mSocket->state() == QAbstractSocket::ConnectedState)
        return;

    do {
        if (mSocket->state() == QAbstractSocket::ConnectedState)
            break;

        qApp->processEvents();
        QThread::msleep(10);
    } while (true);
}

void Client::waitForDisconnected()
{
    if (mSocket->state() == QAbstractSocket::UnconnectedState)
        return;

    do {
        if (mSocket->state() == QAbstractSocket::UnconnectedState)
            break;

        qApp->processEvents();
        QThread::msleep(10);
    } while (true);
}

bool Client::isConnected()
{
    return mSocket->state() == QAbstractSocket::ConnectedState;
}
