#include "chatitemdelegate.h"

#include <QDateTime>
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
    mMarginY = 2;
    mPadding = 6;

    mDateFont.setBold(true);

    mNewFont.setItalic(true);

    mNameFont.setFamily("Arial");
    mNameFont.setPixelSize(10);
    mNameFont.setBold(true);

    mMessageFont.setFamily("Noto Sans");
    mMessageFont.setPixelSize(12);

    mTimeFont.setPixelSize(9);

}

void ChatItemDelegate::setMessageFontPixelSize(int size)
{
    mMessageFont.setPixelSize(size);
}

QSize ChatItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QListView *view = static_cast<QListView *>(option.styleObject);
    QScrollBar *verticalScrollBar = view->verticalScrollBar();
    int width = view->width() - (verticalScrollBar->isVisible()? verticalScrollBar->width() : 0) - 4;
    ChatItemType type = static_cast<ChatItemType>(index.data(ChatItemTypeRole).toInt());

    if (type == ChatItemDate) {
        return QSize(width, (mMarginY * 2) + 24);
    }
    else if (type == ChatItemNew) {
        return QSize(width, (mMarginY * 2) + 24);
    }
    else {
        QString name = index.data(ChatItemNameRole).toString();
        QString msg = index.data(Qt::DisplayRole).toString();
        QDateTime time = index.data(ChatItemTimeRole).toDateTime();

        bool drawName(true);
        if (index.row() > 0) {
            QModelIndex prevIndex = view->model()->index(index.row() - 1, 0);
            drawName = (prevIndex.data(ChatItemNameRole).toString() != name);
        }

        int nameFlags = Qt::TextSingleLine | Qt::AlignVCenter | (type == ChatItemMe? Qt::AlignRight : Qt::AlignLeft);
        int availNameWidth = width - (mMarginX * 2);

        QFontMetrics nameFm(mNameFont);
        QRect nameRect = nameFm.boundingRect(0, 0, availNameWidth, 0, nameFlags, name);

        int timeFlags = Qt::TextSingleLine | Qt::AlignCenter;
        int availTimeWidth = width - (mMarginX * 2);

        QFontMetrics timeFm(mTimeFont);
        QRect timeRect = timeFm.boundingRect(0, 0, availTimeWidth, 0, timeFlags, time.toString("HH:mm"));

        int msgFlags = Qt::TextWordWrap | Qt::AlignVCenter | (type == ChatItemMe? Qt::AlignRight : Qt::AlignLeft);
        int availMsgWidth = (width - (mMarginX * 2)) * 0.85 - (timeRect.width() + (mPadding * 3));

        QFontMetrics msgFm(mMessageFont);
        QRect msgRect = msgFm.boundingRect(0, 0, availMsgWidth, 0, msgFlags, msg);

        return QSize(width, (mMarginY * 2) + (drawName? (nameRect.height() + mMarginY) : 0) + (mPadding * 2) + msgRect.height());
    }
}

void ChatItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // painter->drawRect(0, option.rect.y(), option.rect.width(), option.rect.height());

    QListView *view = static_cast<QListView *>(option.styleObject);
    ChatItemType type = static_cast<ChatItemType>(index.data(ChatItemTypeRole).toInt());

    int width = option.rect.width() - (mMarginX * 2);
    int x = mMarginX;
    int y = option.rect.y() + mMarginY;

    if (type == ChatItemDate) {
        int availWidth = width - (mPadding * 2);
        int dateFlags = Qt::TextSingleLine | Qt::AlignCenter;

        int lineWidth = availWidth * 0.5;
        QString text = index.data(Qt::DisplayRole).toString();

        painter->save();
        painter->setPen(QColor("#555"));
        painter->setFont(mDateFont);
        painter->drawText(mMarginX, y, availWidth, 24, dateFlags, text.toUpper());

        painter->setPen(QColor("#DDD"));
        painter->drawLine(mMarginX + (lineWidth * 0.5), y + 24, mMarginX + (lineWidth * 1.5), y + 24);
        painter->restore();
    }
    else if (type == ChatItemNew) {
        int availWidth = width - (mPadding * 2);
        int newFlags = Qt::TextSingleLine | Qt::AlignCenter;

        QString text = index.data(Qt::DisplayRole).toString();

        painter->save();
        painter->setPen(QColor("#555"));
        painter->setFont(mNewFont);
        painter->drawText(mMarginX, y, availWidth, 24, newFlags, text.toLower());
        painter->restore();
    }
    else {
        QString name = index.data(ChatItemNameRole).toString();
        QString msg = index.data(Qt::DisplayRole).toString();
        QDateTime time = index.data(ChatItemTimeRole).toDateTime();

        bool drawName(true);
        if (index.row() > 0) {
            QModelIndex prevIndex = view->model()->index(index.row() - 1, 0);
            drawName = (prevIndex.data(ChatItemNameRole).toString() != name);
        }

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

        int timeFlags = Qt::TextSingleLine | Qt::AlignCenter;
        int availTimeWidth = width - (mMarginX * 2);

        QFontMetrics timeFm(mTimeFont);
        QRect timeRect = timeFm.boundingRect(0, 0, availTimeWidth, 0, timeFlags, time.toString("HH:mm"));

        int availMsgWidth = width * 0.85 - (timeRect.width() + mPadding * 3);
        int msgFlags = Qt::TextWordWrap | Qt::AlignVCenter | Qt::AlignLeft;

        QFontMetrics msgFm(mMessageFont);
        QRect msgRec = msgFm.boundingRect(0, 0, availMsgWidth, 0, msgFlags, msg);

        int bubbleWidth = msgRec.width() + timeRect.width() + (mPadding * 3);
        int bubbleHeight = msgRec.height() + (mPadding * 2);
        int bubbleX = x + (type == ChatItemMe? (width - bubbleWidth) : 0);
        int bubbleY = y + nameRect.height() + (drawName? mMarginY : 0);

        QColor bubbleColor(type == ChatItemMe? QColor("#DCF8C6") : QColor("#EFEFEF"));
        QColor msgColor(QColor("#111"));
        QColor timeColor(QColor("#888"));

        painter->save();
        painter->setBrush(bubbleColor);
        painter->setPen(QColor("#DDD"));
        painter->drawRoundedRect(bubbleX, bubbleY, bubbleWidth, bubbleHeight, 5, 5);
        painter->restore();

        painter->save();
        painter->setFont(mMessageFont);
        painter->setPen(msgColor);
        painter->drawText(bubbleX + mPadding, bubbleY + mPadding, msgRec.width(), msgRec.height(), msgFlags, msg);
        painter->restore();

        painter->save();
        painter->setFont(mTimeFont);
        painter->setPen(timeColor);
        painter->drawText(bubbleX + msgRec.width() + mPadding * 2, bubbleY + bubbleHeight - timeRect.height() - mPadding, timeRect.width(), timeRect.height(), timeFlags, time.toString("HH:mm"));
        painter->restore();
    }
}
