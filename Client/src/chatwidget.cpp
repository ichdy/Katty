#include "chatwidget.h"

#include <QVBoxLayout>
#include <QTextEdit>
#include <QSplitter>
#include <QLabel>
#include <QListView>
#include <QStandardItemModel>
#include <QDebug>

#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QScrollBar>

#include "engine.h"
#include "shareddata.h"
#include "chatview.h"
#include "chatedit.h"
#include "chatitemdelegate.h"

QHash<QString, ChatWidget *> ChatWidget::mChatWidgetHash;

ChatWidget::ChatWidget(const QString &username, QWidget *parent) :
    QWidget(parent),
    mUsername(username),
    mRequestHistory(false),
    mRequestPending(false)
{   
    QSplitter *splitter = new QSplitter;
    splitter->setOrientation(Qt::Vertical);
    splitter->setChildrenCollapsible(false);

    mChatView = new ChatView;
    mChatEdit = new ChatEdit;

    mChatDelegate = new ChatItemDelegate(mChatView);
    mChatView->setItemDelegate(mChatDelegate);

    mChatView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    QStandardItemModel *model = new QStandardItemModel(mChatView);
    mChatView->setModel(model);

    // Test
    // appendMessage(ChatItemTypeYou, "Probably cats prefer to talk face to face, like this. But for those times when you're far from your chat cat.");
    // appendMessage(ChatItemTypeMe, "The most obvious and common way cats show their happiness and love is through purring. Cats seem to have a special little motor inside them that get started when they are relaxed and enjoying something. You’ll often hear this rumbling, vibrating noise while you are petting your cat. Purrs can also mean your cat is upset but it's not as common.");
    // appendMessage(ChatItemTypeYou, "Children tend to throw themselves on the ground and roll around during a tantrum, but when your cat does it, it means they are excited to see you. Cats may walk or run up to you and throw themselves on the ground and begin to roll around. This is a loving greeting and means they want your attention, especially if they show you their belly.");

    QList<int> sizes;
    sizes << 300;
    sizes << 60;

    mChatEdit->setReturnPressedSignal(true);

    mOfflineLabel = new QLabel("User sedang offline, pesan yang terkirim akan ditampilkan ketika user online.");
    mOfflineLabel->setAlignment(Qt::AlignCenter);
    mOfflineLabel->setWordWrap(true);

    QFont offlineFont = mOfflineLabel->font();
    offlineFont.setBold(true);
    offlineFont.setPixelSize(10);
    mOfflineLabel->setFont(offlineFont);
    mOfflineLabel->setStyleSheet("QLabel { color : darkred; border: 1px solid red; background-color:  #ffffc1; padding: 5px; }");

    QVBoxLayout *topLayout = new QVBoxLayout;
    topLayout->addWidget(mChatView);
    topLayout->addWidget(mOfflineLabel);
    topLayout->setMargin(0);
    topLayout->setSpacing(0);

    QWidget *widget = new QWidget;
    widget->setLayout(topLayout);

    splitter->addWidget(widget);
    splitter->addWidget(mChatEdit);
    splitter->setStretchFactor(0, 1);
    splitter->setSizes(sizes);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(splitter);
    layout->setMargin(3);

    SharedData &sharedData = SharedData::instance();
    mNama = sharedData.userMap[mUsername].nama;
    sharedData.userMap[mUsername].online? mOfflineLabel->hide() : mOfflineLabel->show();

    setLayout(layout);
    setWindowTitle("Chat with " + mNama);
    resize(440, 600);

    connect(Engine::instance(), &Engine::gotData, this, &ChatWidget::onEngineGotData);
    connect(mChatEdit, &ChatEdit::returnPressed, this, &ChatWidget::sendMessage);

    requestChatHistory();
}

ChatWidget::~ChatWidget()
{
    mChatWidgetHash.remove(mUsername);
}

ChatWidget *ChatWidget::getChatWidget(const QString &username)
{
    return mChatWidgetHash[username];
}

ChatWidget *ChatWidget::createChatWidget(const QString &username)
{
    if (mChatWidgetHash[username])
        return mChatWidgetHash[username];

    ChatWidget *widget = new ChatWidget(username);
    widget->setAttribute(Qt::WA_DeleteOnClose, true);
    mChatWidgetHash[username] = widget;
    return widget;
}

void ChatWidget::closeChatWidgets()
{
    QHashIterator<QString, ChatWidget *> iterator(mChatWidgetHash);
    while (iterator.hasNext()) {
        iterator.next();
        iterator.value()->close();
    }
}

void ChatWidget::appendMessage(ChatItemType type, const QString &message, const QDateTime &time)
{
    QStandardItemModel *model = static_cast<QStandardItemModel *>(mChatView->model());

    QDate messageDate = time.date();
    QDate lastMessageDate;
    QStandardItem *lastItem = model->item(model->rowCount() -1, 0);
    if (lastItem)
        lastMessageDate = lastItem->data(ChatItemTimeRole).toDateTime().date();

    if (lastMessageDate.isNull() || lastMessageDate != messageDate) {
        QString dateText;
        if (messageDate == QDate::currentDate())
            dateText = "Hari ini";
        else if (messageDate == QDate::currentDate().addDays(-1))
            dateText = "Kemarin";
        else
            dateText = messageDate.toString("dddd, dd MM yyyy");

        QStandardItem *item = new QStandardItem(dateText);
        item->setData(ChatItemDate, ChatItemTypeRole);
        model->appendRow(item);
    }

    SharedData &sharedData = SharedData::instance();

    QStandardItem *item = new QStandardItem;
    item->setData(type, ChatItemTypeRole);
    item->setData(sharedData.userMap[type == ChatItemMe? sharedData.username : mUsername].nama, ChatItemNameRole);
    item->setData(message, Qt::DisplayRole);
    item->setData(time, ChatItemTimeRole);
    model->appendRow(item);

    mChatView->scrollToBottom();
}

void ChatWidget::requestChatHistory()
{
    mRequestHistory = true;

    QVariantMap request;
    request["type"] = MessagePrivateHistory;
    request["to"] = mUsername;

    Engine::write(request);
}

void ChatWidget::requestChatPending()
{
    if (mRequestHistory)
        mRequestPending = true;
    else {
        QVariantMap request;
        request["type"] = MessagePrivatePending;
        request["from"] = mUsername;

        Engine::write(request);
    }
}

void ChatWidget::sendMessage()
{
    if (mChatEdit->toPlainText().trimmed().isEmpty())
        return;

    QString message = mChatEdit->toPlainText();
    mChatEdit->clear();

    QVariantMap request;
    request["type"] = MessagePrivateChat;
    request["to"] = mUsername;
    request["message"] = message;

    Engine::write(request);
}

void ChatWidget::sendFile()
{
    QString filename = QFileDialog::getOpenFileName(this, "Send File");
    if (filename.isEmpty())
        return;

    /*
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;


    file.close();
    */
}

void ChatWidget::onEngineGotData(const QVariantMap &data)
{
    int type = data["type"].toInt();
    if (type == MessagePrivateChat) {
        QString from = data["from"].toString();
        QString to = data["to"].toString();
        QString message = data["message"].toString();
        QDateTime time = data["time"].toDateTime();

        SharedData &sharedData = SharedData::instance();
        if (from == sharedData.username && to == mUsername) {
            appendMessage(ChatItemMe, message, time);

            if (from == to) {
                QVariantMap respond;
                respond["type"] = MessagePrivateChat;
                respond["id"] = data["id"];
                respond["result"] = true;

                Engine::write(respond);
            }
        }
        else if (from == mUsername && to == sharedData.username) {
            appendMessage(ChatItemYou, message, time);

            QVariantMap respond;
            respond["type"] = MessagePrivateChat;
            respond["id"] = data["id"];
            respond["result"] = true;

            Engine::write(respond);
        }
    }
    else if (type == MessagePrivateHistory) {
        if (data["to"].toString() != mUsername)
            return;

        SharedData &sharedData = SharedData::instance();
        foreach (const QVariant &chat, data["chatList"].toList()) {
            QVariantMap chatMap = chat.toMap();
            QString from = chatMap["from"].toString();
            QString to = chatMap["to"].toString();
            QString message = chatMap["message"].toString();
            QDateTime time = chatMap["time"].toDateTime();

            if (from == sharedData.username && to == mUsername)
                appendMessage(ChatItemMe, message, time);
            else if (from == mUsername && to == sharedData.username)
                appendMessage(ChatItemYou, message, time);
        }

        mRequestHistory = false;
        if (mRequestPending)
            requestChatPending();
    }
    else if (type == MessagePrivatePending) {
        if (data["from"].toString() != mUsername)
            return;

        foreach (const QVariant &chat, data["chatList"].toList()) {
            QVariantMap chatMap = chat.toMap();
            QString message = chatMap["message"].toString();
            QDateTime time = chatMap["time"].toDateTime();

            appendMessage(ChatItemYou, message, time);

            QVariantMap respond;
            respond["type"] = MessagePrivateChat;
            respond["id"] = chatMap["id"];
            respond["result"] = true;

            Engine::write(respond);
        }

        mRequestPending = false;
    }
    else if (type == MessageStatus) {
        QString username = data["username"].toString();
        bool online = data["online"].toBool();

        if (username != mUsername)
            return;

        online? mOfflineLabel->hide() : mOfflineLabel->show();
        mChatView->scrollToBottom();
    }
}
