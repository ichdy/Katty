#include "chatview.h"

ChatView::ChatView(QWidget *parent) :
    QListView(parent) {}

void ChatView::resizeEvent(QResizeEvent *e)
{
    QListView::resizeEvent(e);
    scheduleDelayedItemsLayout();
}
