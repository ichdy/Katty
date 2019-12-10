#ifndef ENGINE_H
#define ENGINE_H

#include <QObject>
#include <QDateTime>

#include "client.h"

class QTimer;
class Engine : public QObject
{
    Q_OBJECT
public:
    static Engine *instance();

    static void write(const QVariantMap &data);

public slots:
    void start();

signals:
    void connected();
    void disconnected();
    void gotData(const QVariantMap &data);

private slots:
    void onConnected();
    void onDisconnected();
    void onBinaryMessageReceived(const QByteArray &data);
    void onTimerTimeout();

private:
    explicit Engine(QObject *parent = nullptr);

    QScopedPointer<Client> mClient;
    QTimer *mTimer;
    QDateTime mAccessTime;
    bool mAccessError;
};

#endif // ENGINE_H
