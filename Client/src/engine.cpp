#include "engine.h"

#include <QVariant>
#include <QTimer>
#include <QJsonDocument>
#include <QDebug>

#include <define.h>

Engine::Engine(QObject *parent) :
    QObject(parent),
    mAccessError(false)
{
    mClient.reset(new Client("Katty-Client"));
    connect(mClient.data(), &Client::connected, this, &Engine::onConnected);
    connect(mClient.data(), &Client::disconnected, this, &Engine::onDisconnected);
    connect(mClient.data(), &Client::binaryMessageReceived, this, &Engine::onBinaryMessageReceived);

    mTimer = new QTimer;
    mTimer->setSingleShot(false);
    mTimer->setInterval(1000);

    connect(mTimer, SIGNAL(timeout()), SLOT(onTimerTimeout()));
}

Engine *Engine::instance()
{
    static Engine engine;
    return &engine;
}

void Engine::write(const QVariantMap &data)
{
    Engine *engine = Engine::instance();
    if (!engine->mClient->isConnected())
        return;

    engine->mClient->send(QJsonDocument::fromVariant(data).toBinaryData());
}

void Engine::start()
{
    mClient->open("127.0.0.1", 8888);
}

void Engine::onConnected()
{
    mAccessTime = QDateTime::currentDateTime();
    mTimer->start();

    emit connected();
}

void Engine::onDisconnected()
{
    mTimer->stop();

    emit disconnected();
    QTimer::singleShot(1000, this, SLOT(start()));
}

void Engine::onBinaryMessageReceived(const QByteArray &data)
{
    mAccessTime = QDateTime::currentDateTime();
    mAccessError = false;

    QVariantMap respond = QJsonDocument::fromBinaryData(data).toVariant().toMap();
    emit gotData(respond);
}

void Engine::onTimerTimeout()
{
    QVariantMap request;
    request["type"] = MessageHeartBeat;
    Engine::write(request);

    if (!mAccessError && QDateTime::currentDateTime() >= mAccessTime.addSecs(10)) {
        mAccessTime = QDateTime::currentDateTime();
        mAccessError = true;
    }
    else if (mAccessError && QDateTime::currentDateTime() >= mAccessTime.addSecs(5))
        mClient->close();
}
