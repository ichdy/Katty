#ifndef CHATVIEW_H
#define CHATVIEW_H

#include <QListView>

class ChatView : public QListView
{
    Q_OBJECT
public:
    explicit ChatView(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *e);
};

#endif // CHATVIEW_H
