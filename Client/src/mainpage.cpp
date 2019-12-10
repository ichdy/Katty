#include "mainpage.h"

#include <QGridLayout>
#include <QTabWidget>
#include <QListView>
#include <QTreeView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QDebug>

#include <define.h>
#include "engine.h"
#include "shareddata.h"
#include "useritemdelegate.h"
#include "chatwidget.h"

MainPage::MainPage(QWidget *parent) :
    QWidget(parent)
{
    mTab = new QTabWidget;
    mPengumumanView = new QListView;
    mLobbyView = new QListView;
    mUserView = new QTreeView;

    mUserView->header()->setStretchLastSection(true);
    mUserView->setHeaderHidden(true);
    mUserView->setEditTriggers(QTreeView::NoEditTriggers);
    mUserView->setItemDelegate(new UserItemDelegate(mUserView));
    mUserView->setMinimumWidth(300);
    mUserView->setMaximumWidth(300);

    mTab->addTab(mPengumumanView, "Pengumuman");
    mTab->addTab(mLobbyView, "Lobby");

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(mTab);
    layout->addWidget(mUserView);

    setLayout(layout);

    Engine *engine = Engine::instance();
    connect(engine, SIGNAL(gotData(QVariantMap)), SLOT(onEngineGotData(QVariantMap)));
    connect(mUserView, &QTreeView::activated, this, &MainPage::onUserItemActivated);
}

void MainPage::loadData()
{
    QVariantMap request;
    request["type"] = MessageDataList;
    Engine::write(request);
}

void MainPage::refreshUserView()
{
    delete mUserView->model();
    QStandardItemModel *model = new QStandardItemModel(mUserView);

    QHash<int, QStandardItem *> seksiItemHash;
    QMapIterator<int, QString> seksiIterator(SharedData::seksiMap());
    while (seksiIterator.hasNext()) {
        seksiIterator.next();
        int id = seksiIterator.key();
        const QString &name = seksiIterator.value();

        QStandardItem *item = new QStandardItem;
        item->setData(name, Qt::DisplayRole);
        item->setData(UserItemSeksi, UserItemTypeRole);
        model->appendRow(item);

        seksiItemHash[id] = item;
    }

    QMap<int, QMap<int, QMap<QString, UserData>>> sortedMap;
    QMapIterator<QString, UserData> userIterator(SharedData::userMap());
    while (userIterator.hasNext()) {
        userIterator.next();
        const UserData &userData = userIterator.value();
        UserData &u = sortedMap[userData.seksi][userData.jabatan][userData.nama];
        u.username = userData.username;
        u.nama = userData.nama;
        u.seksi = userData.seksi;
        u.jabatan = userData.jabatan;
        u.online = userData.online;
    }

    mUserItemHash.clear();
    QMapIterator<int, QMap<int, QMap<QString, UserData>>> sortIterator(sortedMap);
    while (sortIterator.hasNext()) {
        sortIterator.next();
        int seksi = sortIterator.key();

        QMapIterator<int, QMap<QString, UserData>> jabatanIterator(sortIterator.value());
        while (jabatanIterator.hasNext()) {
            jabatanIterator.next();
            int jabatan = jabatanIterator.key();

            QMapIterator<QString, UserData> namaIterator(jabatanIterator.value());
            while (namaIterator.hasNext()) {
                namaIterator.next();
                QString nama = namaIterator.key();

                QStandardItem *item = new QStandardItem;
                item->setData(nama, Qt::DisplayRole);
                item->setData(UserItemUser, UserItemTypeRole);
                item->setData(namaIterator.value().username, UserItemUsernameRole);
                item->setData(SharedData::jabatanMap()[jabatan], UserItemJabatanRole);
                item->setData(namaIterator.value().online, UserItemOnlineRole);

                if (seksiItemHash[seksi])
                    seksiItemHash[seksi]->appendRow(item);

                mUserItemHash[namaIterator.value().username] = item;
            }
        }
    }

    mUserView->setModel(model);
    mUserView->expandAll();
}

void MainPage::onEngineGotData(const QVariantMap &data)
{
    int type = data["type"].toInt();
    if (type == MessageDataList) {
        QVariantList seksiList = data["seksiList"].toList();
        QVariantList jabatanList = data["jabatanList"].toList();
        QVariantList userList = data["userList"].toList();
        QVariantList chatList = data["chatList"].toList();

        SharedData::seksiMap().clear();
        foreach (const QVariant &seksi, seksiList) {
            QVariantMap seksiMap = seksi.toMap();
            int id = seksiMap["id"].toInt();
            QString name = seksiMap["nama"].toString();
            SharedData::seksiMap()[id] = name;
        }

        SharedData::jabatanMap().clear();
        foreach (const QVariant &jabatan, jabatanList) {
            QVariantMap jabatanMap = jabatan.toMap();
            int id = jabatanMap["id"].toInt();
            QString name = jabatanMap["nama"].toString();
            SharedData::jabatanMap()[id] = name;
        }

        SharedData::userMap().clear();
        foreach (const QVariant &user, userList) {
            QVariantMap userMap = user.toMap();
            UserData &userData = SharedData::userMap()[userMap["nip"].toString()];
            userData.username = userMap["nip"].toString();
            userData.nama = userMap["nama"].toString();
            userData.seksi = userMap["seksi"].toInt();
            userData.jabatan = userMap["jabatan"].toInt();
            userData.online = userMap["online"].toBool();
        }

        refreshUserView();

        foreach (const QVariant &chat, chatList) {
            QString from = chat.toString();

            ChatWidget *chatWidget = ChatWidget::getChatWidget(from);
            if (!chatWidget)
                chatWidget = ChatWidget::createChatWidget(from);
            chatWidget->requestChatPending();
            chatWidget->show();
        }
    }
    else if (type == MessageStatus) {
        QString username = data["username"].toString();
        bool online = data["online"].toBool();

        if (mUserItemHash.contains(username)) {
            SharedData::userMap()[username].online = online;
            mUserItemHash[username]->setData(online, UserItemOnlineRole);
        }
    }
    else if (type == MessagePrivateChat) {
        QString from = data["from"].toString();
        QString to = data["to"].toString();

        QString username;
        if (from == SharedData::username())
            username = to;

        if (to == SharedData::username())
            username = from;

        ChatWidget *chatWidget = ChatWidget::getChatWidget(username);
        if (!chatWidget) {
            chatWidget = ChatWidget::createChatWidget(username);
            chatWidget->requestChatPending();
            chatWidget->show();
        }
    }
}

void MainPage::onUserItemActivated(const QModelIndex &index)
{
    if (index.data(UserItemTypeRole).toInt() == UserItemSeksi)
        return;

    QString username = index.data(UserItemUsernameRole).toString();
    ChatWidget *chatWidget = ChatWidget::getChatWidget(username);
    if (!chatWidget)
        chatWidget = ChatWidget::createChatWidget(username);
    chatWidget->show();
}
