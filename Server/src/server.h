#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QHash>
#include <QVariantMap>
#include <QWebSocketServer>
#include <QWebSocket>

typedef QSharedPointer<QWebSocket> QWebSocketPtr;

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(const QString &name, QObject *parent = nullptr);

    void listen(quint16 port);
    QList<QWebSocketPtr> clientList();

signals:
    void listening(bool value);
    void connected(QWebSocketPtr client);
    void disconnected(QWebSocketPtr client);
    void textMessageReceived(QWebSocketPtr client, const QString &text);
    void binaryMessageReceived(QWebSocketPtr client, const QByteArray &data);

private slots:
    void onNewConnection();
    void onDisconnected();
    void onTextMessageReceived(const QString &text);
    void onBinaryMessageReceived(const QByteArray &data);

private:
    QScopedPointer<QWebSocketServer> mSocketServer;
    QHash<QWebSocket *, QWebSocketPtr> mClientHash;
};

#endif // SERVER_H
