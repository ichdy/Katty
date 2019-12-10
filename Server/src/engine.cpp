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
            query.exec("CREATE TABLE `chat` ( `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, `from` VARCHAR(16) NOT NULL, `to` VARCHAR(16) NOT NULL, `time` DATETIME NOT NULL, `message` TEXT NOT NULL, `recieved` INTEGER(1) NOT NULL DEFAULT 0 )");
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

        respond["method"] = method;
        if (!getClient(username)) {
            if (method == LoginUser) {
                QSqlQuery query;
                query.prepare("SELECT * FROM `pegawai` WHERE `nip`=:col0");
                query.bindValue(":col0", username);
                query.exec();

                if (query.next()) {
                    clientDataHash[client].username = username;

                    respond["username"] = username;
                    respond["result"] = true;
                    write(client, respond);

                    QVariantMap status;
                    status["type"] = MessageStatus;
                    status["username"] = username;
                    status["online"] = true;
                    writeToLoggedIn(status);
                }
                else {
                    respond["result"] = false;
                    write(client, respond);
                }
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
                    if (password == _password) {
                        clientDataHash[client].username = username;

                        respond["username"] = username;
                        respond["result"] = true;
                        write(client, respond);

                        QVariantMap status;
                        status["type"] = MessageStatus;
                        status["username"] = username;
                        status["online"] = true;
                        writeToLoggedIn(status);
                    }
                    else {
                        respond["result"] = false;
                        write(client, respond);
                    }
                }
                else {
                    respond["result"] = false;
                    write(client, respond);
                }
            }
        }
        else {
            respond["result"] = false;
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

                QVariantList chatList;
                query.prepare("SELECT DISTINCT `from` FROM `chat` WHERE `to`=:col0 AND `recieved`=0");
                query.bindValue(":col0", clientDataHash[client].username);
                query.exec();
                while (query.next()) {
                    QString from = query.value(0).toString();
                    chatList << from;
                }

                respond["seksiList"] = seksiList;
                respond["jabatanList"] = jabatanList;
                respond["userList"] = userList;
                respond["chatList"] = chatList;
                write(client, respond);
            }
            else if (type == MessagePrivateChat) {
                int id = request["id"].toInt();
                bool result = request["result"].toBool();

                if (id && result) {
                    QSqlQuery query;
                    query.prepare("UPDATE `chat` SET `recieved`=1 WHERE `id`=:col0");
                    query.bindValue(":col0", id);
                    query.exec();
                }
                else {
                    QString from = clientDataHash[client].username;
                    QString to = request["to"].toString();
                    QDateTime time = QDateTime::currentDateTime();
                    QString message = request["message"].toString();
                    bool online = getClient(to);

                    QSqlQuery query;
                    query.exec("SELECT MAX(`id`) FROM `chat`");

                    if (query.next())
                        id = query.value(0).toInt() +1;

                    query.prepare("INSERT INTO `chat`(`id`, `from`, `to`, `time`, `message`) VALUES(:col0, :col1, :col2, :col3, :col4)");
                    query.bindValue(":col0", id);
                    query.bindValue(":col1", from);
                    query.bindValue(":col2", to);
                    query.bindValue(":col3", time);
                    query.bindValue(":col4", message);
                    query.exec();

                    if (from != to) {
                        respond["id"] = id;
                        respond["from"] = from;
                        respond["to"] = to;
                        respond["time"] = time;
                        respond["message"] = message;
                        write(client, respond);
                    }

                    if (online) {
                        QVariantMap forward;
                        forward["type"] = MessagePrivateChat;
                        forward["id"] = id;
                        forward["from"] = from;
                        forward["to"] = to;
                        forward["time"] = time;
                        forward["message"] = message;

                        write(getClient(to), forward);
                    }
                }
            }
            else if (type == MessagePrivateHistory) {
                QString from = clientDataHash[client].username;
                QString to = request["to"].toString();

                QSqlQuery query;
                query.prepare("SELECT * FROM ( SELECT * FROM `chat` WHERE (`from`=:col0 AND `to`=:col1) OR (`from`=:col1 AND `to`=:col0) AND `recieved`=1 ORDER BY `time` DESC LIMIT 50 ) sub ORDER BY `time` ASC");
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

                    QVariantMap temp;
                    temp["id"] = id;
                    temp["from"] = from;
                    temp["to"] = to;
                    temp["time"] = time;
                    temp["message"] = message;

                    chatList << temp;
                }

                respond["to"] = to;
                respond["chatList"] = chatList;
                write(client, respond);
            }
            else if (type == MessagePrivatePending) {
                QString from = request["from"].toString();
                QString to = clientDataHash[client].username;

                QSqlQuery query;
                query.prepare("SELECT * FROM `chat` WHERE `from`=:col0 AND `to`=:col1 AND `recieved`=0 ORDER BY `time` ASC");
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

                    QVariantMap temp;
                    temp["id"] = id;
                    temp["from"] = from;
                    temp["to"] = to;
                    temp["time"] = time;
                    temp["message"] = message;

                    chatList << temp;
                }

                respond["from"] = from;
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
