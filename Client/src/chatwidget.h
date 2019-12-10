#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include "define.h"
#include <QWidget>

class QListView;
class QLabel;
class ChatView;
class ChatItemDelegate;
class ChatEdit;
class ChatWidget : public QWidget
{
    Q_OBJECT
public:
    ~ChatWidget();

    void requestChatHistory();
    void requestChatPending();

    static ChatWidget *getChatWidget(const QString &username);
    static ChatWidget *createChatWidget(const QString &username);
    static void closeChatWidgets();

private slots:
    void sendMessage();
    void sendFile();

    void onEngineGotData(const QVariantMap &data);

private:
    explicit ChatWidget(const QString &username, QWidget *parent = 0);
    void appendMessage(ChatItemType type, const QString &message);

    ChatView *mChatView;
    ChatItemDelegate *mChatDelegate;
    QLabel *mOfflineLabel;
    ChatEdit *mChatEdit;

    QString mUsername;
    QString mNama;

    bool mRequestHistory;
    bool mRequestPending;

    static QHash<QString, ChatWidget *> mChatWidgetHash;
};

#endif // CHATWIDGET_H
