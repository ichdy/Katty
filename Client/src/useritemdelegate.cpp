#include "useritemdelegate.h"

#include <QPainter>
#include <QDebug>

#include <define.h>
#include "shareddata.h"

UserItemDelegate::UserItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

QSize UserItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    if (index.data(UserItemTypeRole).toInt() == UserItemSeksi)
        return QSize(option.rect.width(), 20);

    return QSize(0, 40);
}

void UserItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QFont font = painter->font();
    int y = option.rect.y();
    int width = painter->device()->width();
    int height = option.rect.height();

    painter->save();

    painter->save();
    QColor selectedColor;
    if (option.state & QStyle::State_Selected)
        selectedColor.setNamedColor("#efefef");
    else
        selectedColor.setNamedColor("#fff");

    painter->setPen(selectedColor);
    painter->setBrush(selectedColor);
    painter->drawRect(0, y, width, height);
    painter->restore();

    if (index.data(UserItemTypeRole).toInt() == UserItemSeksi) {
        font.setPixelSize(12);
        font.setWeight(QFont::Bold);
        painter->setFont(font);

        if (option.state & QStyle::State_Open)
            painter->drawText(0, y, 24, height, Qt::AlignCenter, "-");
        else
            painter->drawText(0, y, 24, height, Qt::AlignCenter, "+");

        painter->drawText(22, y + 2, width - 22, 16, Qt::AlignVCenter | Qt::AlignLeft, index.data(Qt::DisplayRole).toString());
    }
    else {
        painter->save();
        if (!(option.state & QStyle::State_Selected))
            painter->setPen(Qt::lightGray);
        painter->drawRect(2, y + 2, width - 5, height - 5);
        painter->restore();

        QString imageFile = index.data(UserItemOnlineRole).toBool()? ":/images/online.png" : ":/images/offline.png";
        painter->drawImage(QRect(8, y + 8, 24, 24), QImage(imageFile));

        painter->setFont(font);
        painter->drawText(40, y + 6, width - 48, 14, Qt::AlignVCenter | Qt::AlignLeft, index.data(Qt::DisplayRole).toString());

        font.setPixelSize(10);
        painter->setFont(font);
        painter->setPen(Qt::darkGray);
        painter->drawText(40, y + 20, width - 48, 14, Qt::AlignVCenter | Qt::AlignLeft, index.data(UserItemJabatanRole).toString());
    }

    painter->restore();
}
