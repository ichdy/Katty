#include "loadingoverlay.h"

#include <QLabel>
#include <QMovie>
#include <QPainter>
#include <QResizeEvent>
#include <QDebug>

LoadingOverlay::LoadingOverlay(QWidget *parent) :
    QWidget(parent)
{
    mMovie = new QMovie(":/images/loading.gif", QByteArray(), this);
    mMovieLabel = new QLabel(this);
    mMovieLabel->setMovie(mMovie);
    mMovie->start();

    mTextLabel = new QLabel(this);
    mTextLabel->setStyleSheet("QLabel { color : #444; }");
}

void LoadingOverlay::setText(const QString &text)
{
    mTextLabel->setText(text);
}

void LoadingOverlay::show(const QString &text)
{
    setText(text);
    QWidget::show();
}

void LoadingOverlay::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QColor backgroundColor = palette().light().color();
    backgroundColor.setAlpha(0);
    QPainter p(this);
    p.fillRect(rect(), backgroundColor);
}

void LoadingOverlay::resizeEvent(QResizeEvent *event)
{
    const QSize &windowSize = event->size();

    QSize imgSize = mMovie->currentImage().size();
    float ratio = imgSize.height() / (float) imgSize.width();

    QFont font("Noto Sans");
    font.setPixelSize(windowSize.height() * 0.35);
    font.setBold(true);
    mTextLabel->setFont(font);

    int imgWidth = qMin(windowSize.width(), windowSize.height());
    int imgHeight = ratio * imgWidth;
    QSize expectedSize(imgWidth, imgHeight);
    mMovie->setScaledSize(expectedSize);
    mMovieLabel->resize(expectedSize);

    mMovieLabel->move(0, (windowSize.height() - imgHeight) / 2);
    mTextLabel->move(imgWidth + 8, (windowSize.height() - mTextLabel->height()) / 2);

    QWidget::resizeEvent(event);
}

void LoadingOverlay::showEvent(QShowEvent *event)
{
    mMovie->start();
    QWidget::showEvent(event);
}

void LoadingOverlay::hideEvent(QHideEvent *event)
{
    mMovie->stop();
    QWidget::hideEvent(event);
}
