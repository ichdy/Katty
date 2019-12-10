#ifndef ENGINE_H
#define ENGINE_H

#include <QObject>
#include <QDateTime>
#include <QHash>
#include "server.h"

template <class T>
uint qHash( const QSharedPointer<T> &p )
{
     return qHash( p.data() );
}

struct ClientData {
    QDateTime accessTime;
    QString username;
};

typedef struct ClientData ClientData;

class QTimer;
class Engine : public QObject
{
    Q_OBJECT
public:

    static Engine *instance();
    void start();

    QHash<QWebSocketPtr, ClientData> clientDataHash;

    static void write(QWebSocketPtr socket, const QVariantMap &data);
    static void writeToLoggedIn(const QVariantMap &data);
    static QWebSocketPtr getClient(const QString &username);

signals:
    void listening(bool value);

private slots:
    void onListening(bool value);
    void onConnected(QWebSocketPtr client);
    void onDisconnected(QWebSocketPtr client);
    void onBinaryMessageReceived(QWebSocketPtr client, const QByteArray &data);
    void onTimerTimeout();

private:
    explicit Engine(QObject *parent = nullptr);

    QScopedPointer<Server> mServer;
    QTimer *mTimer;
};

#endif // ENGINE_H
