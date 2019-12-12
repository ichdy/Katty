#include "chatwidget.h"

#include <QEvent>
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
    mNewMessageItem(nullptr),
    mUsername(username)
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

bool ChatWidget::event(QEvent *event)
{
    if (event->type() == QEvent::WindowActivate)
        QTimer::singleShot(5000, this, &ChatWidget::removeNewMessageItem);
    return QWidget::event(event);
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

void ChatWidget::removeNewMessageItem()
{
    if (!mNewMessageItem)
        return;

    QStandardItemModel *model = static_cast<QStandardItemModel *>(mChatView->model());
    model->removeRow(mNewMessageItem->row());

    mNewMessageItem = nullptr;
}

void ChatWidget::appendMessage(ChatItemType type, int id, const QString &message, const QDateTime &time, bool isNew)
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

    if (type == ChatItemYou && isNew && !mNewMessageItem) {
        mNewMessageItem = new QStandardItem("new message(s)");
        mNewMessageItem->setData(ChatItemNew, ChatItemTypeRole);
        model->appendRow(mNewMessageItem);
    }

    SharedData &sharedData = SharedData::instance();

    QStandardItem *item = new QStandardItem;
    item->setData(type, ChatItemTypeRole);
    item->setData(id, ChatItemIdRole);
    item->setData(sharedData.userMap[type == ChatItemMe? sharedData.username : mUsername].nama, ChatItemNameRole);
    item->setData(message, Qt::DisplayRole);
    item->setData(time, ChatItemTimeRole);
    item->setData(isNew, ChatItemStatusRole);
    model->appendRow(item);

    mChatView->scrollToBottom();
}

void ChatWidget::requestChatHistory()
{
    QVariantMap request;
    request["type"] = MessagePrivateHistory;
    request["to"] = mUsername;
    Engine::write(request);
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
        int id = data["id"].toInt();
        QString from = data["from"].toString();
        QString to = data["to"].toString();
        QString message = data["message"].toString();
        QDateTime time = data["time"].toDateTime();
        bool isNew = data["new"].toBool();

        ChatItemType chatType(ChatItemUnknown);
        QString username;
        if (to == SharedData::instance().username) {
            username = from;
            chatType = ChatItemYou;
        }

        if (from == SharedData::instance().username) {
            username = to;
            chatType = ChatItemMe;
        }

        if (username != mUsername)
            return;

        appendMessage(chatType, id, message, time, isNew);

        QVariantMap respond;
        respond["type"] = MessagePrivateChatStatus;
        respond["id"] = id;
        respond["received"] = true;
        Engine::write(respond);
    }
    else if (type == MessagePrivateHistory) {
        if (data["to"].toString() != mUsername)
            return;

        foreach (const QVariant &chat, data["chatList"].toList()) {
            QVariantMap chatMap = chat.toMap();
            int id = chatMap["id"].toInt();
            QString from = chatMap["from"].toString();
            QString to = chatMap["to"].toString();
            QString message = chatMap["message"].toString();
            QDateTime time = chatMap["time"].toDateTime();
            bool isNew = chatMap["new"].toBool();

            ChatItemType chatType(ChatItemUnknown);
            if (to == SharedData::instance().username)
                chatType = ChatItemYou;

            if (from == SharedData::instance().username)
                chatType = ChatItemMe;

            appendMessage(chatType, id, message, time, isNew);

            if (isNew) {
                QVariantMap respond;
                respond["type"] = MessagePrivateChatStatus;
                respond["id"] = id;
                respond["received"] = true;
                Engine::write(respond);
            }
        }
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
