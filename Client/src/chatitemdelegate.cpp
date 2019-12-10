#include "chatitemdelegate.h"

#include <QFontMetrics>
#include <QPainter>
#include <QListView>
#include <QScrollBar>
#include <QDebug>

#include <define.h>
#include "shareddata.h"

ChatItemDelegate::ChatItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
    mMarginX = 8;
    mMarginY = 1;
    mPadding = 6;

    mNameFont.setFamily("Arial");
    mNameFont.setPixelSize(10);
    mNameFont.setBold(true);

    mMessageFont.setFamily("Noto Sans");
    mMessageFont.setPixelSize(12);
}

void ChatItemDelegate::setMessageFontPixelSize(int size)
{
    mMessageFont.setPixelSize(size);
}

QSize ChatItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString name = index.data(ChatItemNameRole).toString();
    QString msg = index.data(Qt::DisplayRole).toString();

    QListView *view = static_cast<QListView *>(option.styleObject);
    QScrollBar *verticalScrollBar = view->verticalScrollBar();

    bool drawName(true);
    if (index.row() > 0) {
        QModelIndex prevIndex = view->model()->index(index.row() - 1, 0);
        drawName = (prevIndex.data(ChatItemNameRole).toString() != name);
    }

    int type = index.data(ChatItemTypeRole).toInt();
    int width = view->width() - (verticalScrollBar->isVisible()? verticalScrollBar->width() : 0) - 4;

    int nameFlags = Qt::TextSingleLine | Qt::AlignVCenter | (type == ChatItemMe? Qt::AlignRight : Qt::AlignLeft);
    int availNameWidth = width - (mMarginX * 2);

    QFontMetrics nameFm(mNameFont);
    QRect nameRect = nameFm.boundingRect(0, 0, availNameWidth, 0, nameFlags, name);

    int availMsgWidth = (width - (mMarginX * 2)) * 0.85 - (mPadding * 2);
    int msgFlags = Qt::TextWordWrap | Qt::AlignVCenter | (type == ChatItemMe? Qt::AlignRight : Qt::AlignLeft);

    QFontMetrics msgFm(mMessageFont);
    QRect msgRect = msgFm.boundingRect(0, 0, availMsgWidth, 0, msgFlags, msg);

    return QSize(width, (mMarginY * 2) + (drawName? nameRect.height() : 0) + (mPadding * 2) + msgRect.height());
}

void ChatItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString name = index.data(ChatItemNameRole).toString();
    QString msg = index.data(Qt::DisplayRole).toString();

    QListView *view = static_cast<QListView *>(option.styleObject);

    bool drawName(true);
    if (index.row() > 0) {
        QModelIndex prevIndex = view->model()->index(index.row() - 1, 0);
        drawName = (prevIndex.data(ChatItemNameRole).toString() != name);
    }

    int x = mMarginX;
    int y = option.rect.y() + mMarginY;
    int width = option.rect.width() - (mMarginX * 2);

    // painter->drawRect(0, option.rect.y(), option.rect.width(), option.rect.height());

    int type = index.data(ChatItemTypeRole).toInt();
    QRect nameRect;

    if (drawName) {
        int availNameWidth = width - (mMarginX * 2);
        int nameFlags = Qt::TextSingleLine | Qt::AlignVCenter | (type == ChatItemMe? Qt::AlignRight : Qt::AlignLeft);

        QFontMetrics nameFm(mNameFont);
        nameRect = nameFm.boundingRect(0, 0, availNameWidth, 0, nameFlags, name);

        int nameWidth = nameRect.width();
        int nameHeight = nameRect.height();
        int nameX = x + (type == ChatItemMe? (width - nameWidth) : 0);
        int nameY = y;

        QColor nameColor("#000");

        painter->save();
        painter->setFont(mNameFont);
        painter->setPen(nameColor);
        painter->drawText(nameX, nameY, nameWidth, nameHeight, nameFlags, name);
        painter->restore();
    }

    int availMsgWidth = width * 0.85 - (mPadding * 2);
    int msgFlags = Qt::TextWordWrap | Qt::AlignVCenter | (type == ChatItemMe? Qt::AlignRight : Qt::AlignLeft);

    QFontMetrics msgFm(mMessageFont);
    QRect msgRet = msgFm.boundingRect(0, 0, availMsgWidth, 0, msgFlags, msg);

    int bubbleWidth = msgRet.width() + (mPadding * 2);
    int bubbleHeight = msgRet.height() + (mPadding * 2);
    int bubbleX = x + (type == ChatItemMe? (width - bubbleWidth) : 0);
    int bubbleY = y + nameRect.height();

    QColor bubbleColor(type == ChatItemMe? QColor("#27ABFF") : QColor("#EFEFEF"));
    QColor msgColor(type == ChatItemMe? QColor("#FFF") : QColor("#222"));

    painter->save();
    painter->setBrush(bubbleColor);
    painter->setPen(Qt::transparent);
    painter->drawRoundedRect(bubbleX, bubbleY, bubbleWidth, bubbleHeight, 5, 5);
    painter->restore();

    painter->save();
    painter->setFont(mMessageFont);
    painter->setPen(msgColor);
    painter->drawText(bubbleX + mPadding, bubbleY + mPadding, msgRet.width(), msgRet.height(), msgFlags, msg);
    painter->restore();
}
