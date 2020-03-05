#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QScopedPointer>
#include <QTimer>
#include <QImage>
#include <QWebSocket>

class QWebSocket;
class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(const QString &name, QObject *parent = nullptr);

    inline QString name() { return mName; }
    inline QString host() { return mHost; }
    inline quint16 port() { return mPort; }

    void open(const QString &host, quint16 port);
    void close();
    bool isConnected();
    void send(const QByteArray &data);

    void waitForConnected();
    void waitForDisconnected();

signals:
    void connected();
    void disconnected();
    void binaryMessageReceived(const QByteArray &msg);

private:
    QScopedPointer<QWebSocket> mSocket;
    QString mName;
    QString mHost;
    quint16 mPort;
};

#endif // CLIENT_H
