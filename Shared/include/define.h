#ifndef MESSAGE_H
#define MESSAGE_H

#include <qnamespace.h>

enum MessageType {
    MessageUnknown = 0,
    MessageHeartBeat,
    MessageLogin,
    MessageLogout,
    MessageDataList,
    MessageStatus,
    MessagePrivateChat,
    MessagePrivateChatStatus,
    MessagePrivateSendFile,
    MessagePrivateHistory,
    MessagePrivatePending,
    MessageGroupChat,
    MessageGroupSendFile,
    MessageGroupHistory,
    MessageBroadcast,
    MessagePengumuman
};

enum LoginType {
    LoginUnknown = 0,
    LoginUser,
    LoginPassword
};

enum UserItemRole {
    UserItemTypeRole = Qt::UserRole + 1,
    UserItemUsernameRole,
    UserItemNameRole,
    UserItemSeksiRole,
    UserItemJabatanRole,
    UserItemOnlineRole
};

enum UserItemType {
    UserItemUnknown = 0,
    UserItemSeksi,
    UserItemUser
};

enum ChatItemRole {
    ChatItemTypeRole = Qt::UserRole + 1,
    ChatItemIdRole,
    ChatItemNameRole,
    ChatItemTimeRole,
    ChatItemStatusRole
};

enum ChatItemType {
    ChatItemUnknown = 0,
    ChatItemMe,
    ChatItemYou,
    ChatItemDate,
    ChatItemNew
};

#endif
