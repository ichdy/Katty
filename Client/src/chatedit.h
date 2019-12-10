#ifndef CHATEDIT_H
#define CHATEDIT_H

#include <QTextEdit>

class ChatEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit ChatEdit(QWidget *parent = 0);

    void setReturnPressedSignal(bool enable);

signals:
    void returnPressed();

protected:
    void keyPressEvent(QKeyEvent *e);

private:
    bool mEnterPressed;
};

#endif // CHATEDIT_H
