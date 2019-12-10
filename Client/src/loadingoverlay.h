#ifndef LOADINGOVERLAY_H
#define LOADINGOVERLAY_H

#include <QWidget>

class QLabel;
class LoadingOverlay : public QWidget
{
    Q_OBJECT
public:
    LoadingOverlay(QWidget *parent=0);

    void setText(const QString &text);
    void show(const QString &text);

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);

private:
    QMovie *mMovie;
    QLabel *mMovieLabel;
    QLabel *mTextLabel;
};

#endif // LOADINGOVERLAY_H
