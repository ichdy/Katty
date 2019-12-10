#include "mainwindow.h"

#include <QDebug>
#include <QJsonDocument>
#include <QTimer>
#include <QLabel>

#include "engine.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    mLabel = new QLabel;
    mLabel->setAlignment(Qt::AlignCenter);
    setCentralWidget(mLabel);

    Engine *engine = Engine::instance();
    connect(engine, &Engine::listening, this, &MainWindow::onListening);
}

void MainWindow::onListening(bool value)
{
    mLabel->setText(QString("Listening: %1").arg(value? "OK" : "NOT OK"));
}
