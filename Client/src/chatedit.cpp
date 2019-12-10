#include "chatedit.h"

#include <QKeyEvent>

ChatEdit::ChatEdit(QWidget *parent) :
    QTextEdit(parent),
    mEnterPressed(false)
{
    setAcceptRichText(false);
}

void ChatEdit::setReturnPressedSignal(bool enable)
{
    mEnterPressed = enable;
}

void ChatEdit::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (mEnterPressed) {
            emit returnPressed();
            return;
        }
        break;
    }

    QTextEdit::keyPressEvent(e);
}
