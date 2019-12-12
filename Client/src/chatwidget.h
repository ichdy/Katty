#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include "define.h"
#include <QWidget>

class QListView;
class QLabel;
class QStandardItem;
class ChatView;
class ChatItemDelegate;
class ChatEdit;
class ChatWidget : public QWidget
{
    Q_OBJECT
public:
    ~ChatWidget();

    void appendMessage(ChatItemType type, int id, const QString &message, const QDateTime &time, bool isNew);

    static ChatWidget *getChatWidget(const QString &username);
    static ChatWidget *createChatWidget(const QString &username);
    static void closeChatWidgets();

protected:
    bool event(QEvent *event);

private slots:
    void sendMessage();
    void sendFile();
    void removeNewMessageItem();

    void onEngineGotData(const QVariantMap &data);

private:
    explicit ChatWidget(const QString &username, QWidget *parent = 0);
    void requestChatHistory();

    ChatView *mChatView;
    ChatItemDelegate *mChatDelegate;
    QLabel *mOfflineLabel;
    ChatEdit *mChatEdit;
    QStandardItem *mNewMessageItem;

    QString mUsername;
    QString mNama;

    static QHash<QString, ChatWidget *> mChatWidgetHash;
};

#endif // CHATWIDGET_H
