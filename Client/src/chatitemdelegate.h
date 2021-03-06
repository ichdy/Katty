#ifndef CHATITEMDELEGATE_H
#define CHATITEMDELEGATE_H

#include <QStyledItemDelegate>

class ChatItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ChatItemDelegate(QObject *parent = 0);

    void setMessageFontPixelSize(int size);

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    int mMarginX, mMarginY;
    int mPadding;
    QFont mDateFont;
    QFont mNewFont;
    QFont mNameFont;
    QFont mMessageFont;
    QFont mTimeFont;
};

#endif // CHATITEMDELEGATE_H
