#ifndef USERITEMDELEGATE_H
#define USERITEMDELEGATE_H

#include <QStyledItemDelegate>

class UserItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit UserItemDelegate(QObject *parent = 0);

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // USERITEMDELEGATE_H
