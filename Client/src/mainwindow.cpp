#include "mainwindow.h"

#include <QResizeEvent>
#include <QGridLayout>
#include <QLabel>
#include <QTimer>
#include <QStackedWidget>
#include <QTabWidget>
#include <QListView>
#include <QDebug>

#include <define.h>
#include "engine.h"
#include "loadingoverlay.h"
#include "loginpage.h"
#include "mainpage.h"
#include "shareddata.h"
#include "chatwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    mStack = new QStackedWidget(this);
    mLoginPage = new LoginPage;
    mMainPage = new MainPage;

    mLoading = new LoadingOverlay(this);
    mLoading->resize(200, 48);
    mLoading->hide();

    mStack->addWidget(mLoginPage);
    mStack->addWidget(mMainPage);

    Engine *engine = Engine::instance();
    connect(engine, SIGNAL(connected()), SLOT(onEngineConnected()));
    connect(engine, SIGNAL(disconnected()), SLOT(onEngineDisconnected()));
    connect(engine, SIGNAL(gotData(QVariantMap)), SLOT(onEngineGotData(QVariantMap)));

    setWindowIcon(QIcon(":/images/logo.png"));
    resize(960, 620);

    onEngineDisconnected();
    engine->start();
}

MainWindow::~MainWindow() {}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QSize windowSize = event->size();
    QSize loadingSize = mLoading->size();

    mStack->resize(windowSize);

    int margin = 15;
    int x = margin;
    int y = windowSize.height() - margin - loadingSize.height();
    mLoading->move(x, y);

    QMainWindow::resizeEvent(event);
}

void MainWindow::onEngineConnected()
{
    mLoading->hide();
}

void MainWindow::onEngineDisconnected()
{
    mStack->setCurrentWidget(mLoginPage);
    mLoading->show("Connecting...");
    ChatWidget::closeChatWidgets();
}

void MainWindow::onEngineGotData(const QVariantMap &data)
{
    int type = data["type"].toInt();
    if (type == MessageLogin) {
        if (data["online"].toBool()) {
            QString username = data["username"].toString();
            SharedData::instance().username = username;

            mStack->setCurrentWidget(mMainPage);
            mMainPage->loadData();
            mMainPage->loadPendingChat();
        }
    }
    else if (type == MessageLogout) {
        bool result = data["result"].toBool();
        if (result) {
            SharedData::instance().username.clear();
            mStack->setCurrentWidget(mLoginPage);
            ChatWidget::closeChatWidgets();
        }
    }
}
