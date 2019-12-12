#include "engine.h"

#include <QTimer>
#include <QJsonDocument>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <define.h>

Engine::Engine(QObject *parent) :
    QObject(parent)
{
    mServer.reset(new Server("Katty-Server"));
    connect(mServer.data(), &Server::listening, this, &Engine::onListening);
    connect(mServer.data(), &Server::connected, this, &Engine::onConnected);
    connect(mServer.data(), &Server::disconnected, this, &Engine::onDisconnected);
    connect(mServer.data(), &Server::binaryMessageReceived, this, &Engine::onBinaryMessageReceived);

    mTimer = new QTimer;
    mTimer->setSingleShot(false);
    mTimer->setInterval(1000);
    mTimer->start();

    connect(mTimer, SIGNAL(timeout()), SLOT(onTimerTimeout()));
}

Engine *Engine::instance()
{
    static Engine engine;
    return &engine;
}

void Engine::write(QWebSocketPtr socket, const QVariantMap &data)
{
    socket->sendBinaryMessage(QJsonDocument::fromVariant(data).toBinaryData());
}

void Engine::writeToLoggedIn(const QVariantMap &data)
{
    QHashIterator<QWebSocketPtr, ClientData> iterator(Engine::instance()->clientDataHash);
    while (iterator.hasNext()) {
        iterator.next();
        if (!iterator.value().username.isEmpty())
            write(iterator.key(), data);
    }
}

QWebSocketPtr Engine::getClient(const QString &username)
{
    QMutableHashIterator<QWebSocketPtr, ClientData> iterator(Engine::instance()->clientDataHash);
    while (iterator.hasNext()) {
        iterator.next();
        ClientData &data = iterator.value();
        if (data.username == username)
            return iterator.key();
    }

    return QWebSocketPtr();
}

void Engine::start()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("katty.db");

    if (db.open()) {
        QSqlQuery query;
        query.exec("SELECT `value` FROM `settings` WHERE `key`='version'");
        QString version;
        if (query.next())
            version = query.value(0).toString();

        if (version.isEmpty()) {
            query.exec("CREATE TABLE `chats` ( `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, `from` VARCHAR(16) NOT NULL, `to` VARCHAR(16) NOT NULL, `time` DATETIME NOT NULL, `message` TEXT NOT NULL, `received` INTEGER(1) NOT NULL DEFAULT 0 )");
            query.exec("CREATE TABLE `seksi` ( `id` INTEGER NOT NULL, `nama` TEXT NOT NULL )");
            query.exec("CREATE TABLE `jabatan` ( `id` INTEGER NOT NULL, `nama` TEXT NOT NULL )");
            query.exec("CREATE TABLE `pegawai` ( `nip` VARCHAR(9) NOT NULL, `nama` TEXT NOT NULL, `seksi` INTEGER NOT NULL, `jabatan` INTEGER NOT NULL )");
            query.exec("CREATE TABLE `users` ( `username` VARCHAR(16) NOT NULL, `password` VARCHAR(128) NOT NULL )");
            query.exec("INSERT INTO `settings` VALUES('version', '1')");
            version = "1";
        }

        /*
        if (version == "1") {
            qDebug() << "updating database v2...";
            query.exec("CREATE TABLE `pegawai` ( `nip` VARCHAR(9) NOT NULL, `nama` TEXT NOT NULL, `eksten` INTEGER NOT NULL DEFAULT 0 )");
            query.exec("UPDATE `settings` SET `value`='2' WHERE `key`='version'");
            version = "2";
        }
        */
    }

    mServer->listen(8888);
}

void Engine::onListening(bool value)
{
    emit listening(value);
}

void Engine::onConnected(QWebSocketPtr client)
{
    clientDataHash[client].accessTime = QDateTime::currentDateTime();
}

void Engine::onDisconnected(QWebSocketPtr client)
{
    QString username = clientDataHash[client].username;
    if (!username.isEmpty()) {
        QVariantMap status;
        status["type"] = MessageStatus;
        status["username"] = username;
        status["online"] = false;
        writeToLoggedIn(status);
    }

    clientDataHash.remove(client);
}

void Engine::onBinaryMessageReceived(QWebSocketPtr client, const QByteArray &data)
{
    clientDataHash[client].accessTime = QDateTime::currentDateTime();

    QVariantMap request = QJsonDocument::fromBinaryData(data).toVariant().toMap();
    int type = request["type"].toInt();

    QVariantMap respond;
    respond["type"] = type;

    if (type == MessageHeartBeat) {
        write(client, respond);
    }
    else if (type == MessageLogin) {
        int method = request["method"].toInt();
        QString username = request["username"].toString();
        bool success(false);

        respond["method"] = method;
        if (method == LoginUser) {
            QSqlQuery query;
            query.prepare("SELECT * FROM `pegawai` WHERE `nip`=:col0");
            query.bindValue(":col0", username);
            query.exec();

            if (query.next())
                success = true;
        }
        else {
            QString username = request["username"].toString();
            QString password = request["password"].toString();

            QSqlQuery query;
            query.prepare("SELECT `password` FROM `users` WHERE `username`=:col0");
            query.bindValue(":col0", username);
            query.exec();

            if (query.next()) {
                QString _password = query.value(0).toString();
                if (password == _password)
                    success = true;
            }
        }

        if (success) {
            clientDataHash[client].username = username;

            respond["username"] = username;
            respond["online"] = true;
            write(client, respond);

            QVariantMap status;
            status["type"] = MessageStatus;
            status["username"] = username;
            status["online"] = true;
            writeToLoggedIn(status);
        }
        else {
            respond["online"] = false;
            write(client, respond);
        }
    }
    else if (type == MessageLogout) {
        QString username = clientDataHash[client].username;
        clientDataHash[client].username.clear();

        respond["result"] = true;
        write(client, respond);

        QVariantMap status;
        status["type"] = MessageStatus;
        status["username"] = username;
        status["online"] = false;
        writeToLoggedIn(status);
    }
    else {
        if (!clientDataHash[client].username.isEmpty()) {
            if (type == MessageDataList) {
                QSqlQuery query;

                QVariantList seksiList;
                query.exec("SELECT * FROM `seksi`");
                while (query.next()) {
                    int id = query.value(0).toInt();
                    QString nama = query.value(1).toString();

                    QVariantMap temp;
                    temp["id"] = id;
                    temp["nama"] = nama;
                    seksiList << temp;
                }

                QVariantList jabatanList;
                query.exec("SELECT * FROM `jabatan`");
                while (query.next()) {
                    int id = query.value(0).toInt();
                    QString nama = query.value(1).toString();

                    QVariantMap temp;
                    temp["id"] = id;
                    temp["nama"] = nama;
                    jabatanList << temp;
                }

                QVariantList userList;
                query.exec("SELECT * FROM `pegawai`");
                while (query.next()) {
                    QString nip = query.value(0).toString();
                    QString nama = query.value(1).toString();
                    int seksi = query.value(2).toInt();
                    int jabatan = query.value(3).toInt();
                    bool online = getClient(nip);

                    QVariantMap temp;
                    temp["nip"] = nip;
                    temp["nama"] = nama;
                    temp["seksi"] = seksi;
                    temp["jabatan"] = jabatan;
                    temp["online"] = online;
                    userList << temp;
                }

                respond["seksiList"] = seksiList;
                respond["jabatanList"] = jabatanList;
                respond["userList"] = userList;
                write(client, respond);
            }
            else if (type == MessagePrivateChat) {
                QString from = clientDataHash[client].username;
                QString to = request["to"].toString();
                QDateTime time = QDateTime::currentDateTime();
                QString message = request["message"].toString();
                bool online = getClient(to);

                QSqlQuery query;
                query.exec("SELECT MAX(`id`) FROM `chats`");

                int id(0);
                if (query.next())
                    id = query.value(0).toInt() +1;

                query.prepare("INSERT INTO `chats` VALUES(:col0, :col1, :col2, :col3, :col4, :col5)");
                query.bindValue(":col0", id);
                query.bindValue(":col1", from);
                query.bindValue(":col2", to);
                query.bindValue(":col3", time);
                query.bindValue(":col4", message);
                query.bindValue(":col5", 0);
                query.exec();

                respond["id"] = id;
                respond["from"] = from;
                respond["to"] = to;
                respond["time"] = time;
                respond["message"] = message;
                respond["new"] = true;

                if (from != to)
                    write(client, respond);

                if (online)
                    write(getClient(to), respond);
            }
            else if (type == MessagePrivateChatStatus) {
                int id = request["id"].toInt();
                QString to = clientDataHash[client].username;
                bool received = request["received"].toBool();

                QSqlQuery query;
                query.prepare("SELECT `to` FROM `chats` WHERE `id`=:col0");
                query.bindValue(":col0", id);
                query.exec();

                QString _to;
                if (query.next())
                    _to = query.value(0).toString();

                if (_to == to) {
                    query.prepare("UPDATE `chats` SET `received`=:col1 WHERE `id`=:col0");
                    query.bindValue(":col0", id);
                    query.bindValue(":col1", received);
                    query.exec();
                }
            }
            else if (type == MessagePrivateHistory) {
                QString from = clientDataHash[client].username;
                QString to = request["to"].toString();

                QSqlQuery query;
                query.prepare("SELECT * FROM ( SELECT * FROM `chats` WHERE (`from`=:col0 AND `to`=:col1) OR (`from`=:col1 AND `to`=:col0) ORDER BY `time` DESC LIMIT 50 ) sub ORDER BY `time` ASC");
                query.bindValue(":col0", from);
                query.bindValue(":col1", to);
                query.exec();

                QVariantList chatList;
                while (query.next()) {
                    int id = query.value(0).toInt();
                    QString from = query.value(1).toString();
                    QString to = query.value(2).toString();
                    QDateTime time = query.value(3).toDateTime();
                    QString message = query.value(4).toString();
                    bool isNew = !query.value(5).toBool();

                    QVariantMap temp;
                    temp["id"] = id;
                    temp["from"] = from;
                    temp["to"] = to;
                    temp["time"] = time;
                    temp["message"] = message;
                    temp["new"] = isNew;

                    chatList << temp;
                }

                respond["to"] = to;
                respond["chatList"] = chatList;
                write(client, respond);
            }
            else if (type == MessagePrivatePending) {
                QString to = clientDataHash[client].username;

                QSqlQuery query;
                query.prepare("SELECT `from` FROM `chats` WHERE `to`=:col0 AND `received`=0 GROUP BY `from`");
                query.bindValue(":col0", to);
                query.exec();

                QStringList chatList;
                while (query.next())
                    chatList << query.value(0).toString();

                respond["chatList"] = chatList;
                write(client, respond);
            }
        }
    }
}

void Engine::onTimerTimeout()
{
    QHashIterator<QWebSocketPtr, ClientData> iterator(clientDataHash);
    while (iterator.hasNext()) {
        iterator.next();
        const ClientData &data = iterator.value();

        if (QDateTime::currentDateTime() >= data.accessTime.addSecs(20))
            iterator.key()->close();
    }
}
