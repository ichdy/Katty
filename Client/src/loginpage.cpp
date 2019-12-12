#include "loginpage.h"

#include <QVariant>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QPainter>
#include <QPaintEvent>
#include <QMessageBox>
#include <QDebug>

#include <define.h>
#include "engine.h"

LoginPage::LoginPage(QWidget *parent) :
    QWidget(parent)
{
    QFont font("Noto Sans");

    mIconLabel = new QLabel;
    mUsernameEdit = new QLineEdit;
    mPasswordEdit = new QLineEdit;

    mIconLabel->setPixmap(QPixmap(":/images/logo.png"));
    mIconLabel->setMinimumWidth(200);
    mIconLabel->setMinimumHeight(200);
    mIconLabel->setAlignment(Qt::AlignCenter);

    mUsernameEdit->setMinimumHeight(28);
    mPasswordEdit->setMinimumHeight(28);

    font.setPixelSize(16);
    mUsernameEdit->setFont(font);
    mPasswordEdit->setFont(font);

    mPasswordEdit->setEchoMode(QLineEdit::Password);

    mLoginButton = new QPushButton("LOGIN");
    mLoginButton->setMinimumHeight(36);

    QLabel *usernameLabel = new QLabel("Username");
    QLabel *passwordLabel = new QLabel("Password");

    font.setPixelSize(14);
    usernameLabel->setFont(font);
    passwordLabel->setFont(font);
    mLoginButton->setFont(font);

    QGridLayout *formLayout = new QGridLayout;
    formLayout->addWidget(mIconLabel, 0, 0, 1, 2);
    formLayout->addWidget(usernameLabel, 1, 0);
    formLayout->addWidget(passwordLabel, 2, 0);
    formLayout->addWidget(mUsernameEdit, 1, 1);
    formLayout->addWidget(mPasswordEdit, 2, 1);
    formLayout->addWidget(mLoginButton, 3, 0, 1, 2);

    QWidget *form = new QWidget;
    form->setMaximumWidth(250);
    form->setMaximumHeight(340);
    form->setLayout(formLayout);

    QGridLayout *layout = new QGridLayout;
    layout->addItem(new QSpacerItem(100, 100, QSizePolicy::Expanding, QSizePolicy::Expanding), 0, 1);
    layout->addItem(new QSpacerItem(100, 100, QSizePolicy::Expanding, QSizePolicy::Expanding), 1, 0);
    layout->addItem(new QSpacerItem(100, 100, QSizePolicy::Expanding, QSizePolicy::Expanding), 1, 2);
    layout->addItem(new QSpacerItem(100, 100, QSizePolicy::Expanding, QSizePolicy::Expanding), 2, 1);
    layout->addWidget(form, 1, 1);

    setLayout(layout);
    enableForm(false);

    Engine *engine = Engine::instance();
    connect(engine, SIGNAL(connected()), SLOT(onEngineConnected()));
    connect(engine, SIGNAL(disconnected()), SLOT(onEngineDisconnected()));
    connect(engine, SIGNAL(gotData(QVariantMap)), SLOT(onEngineGotData(QVariantMap)));
    connect(mLoginButton, SIGNAL(clicked(bool)), SLOT(loginByPassword()));
}

void LoginPage::paintEvent(QPaintEvent *event)
{
    QImage bgImage(":/images/bg.png");
    int scrWidth = width();
    int scrHeight = height();
    int bgWidth = bgImage.width();
    int bgHeight = bgImage.height();

    int x(0), y(0);
    QPainter p(this);
    while (x < scrWidth) {
        while (y < scrHeight) {
            p.drawImage(x, y, bgImage);
            y += bgHeight;
        }
        x += bgWidth;
        y = 0;
    }
    p.end();

    QWidget::paintEvent(event);
}

void LoginPage::enableForm(bool value)
{
    mUsernameEdit->setEnabled(value);
    mPasswordEdit->setEnabled(value);
    mLoginButton->setEnabled(value);
}

void LoginPage::clearForm()
{
    mUsernameEdit->clear();
    mPasswordEdit->clear();
}

void LoginPage::loginByUser()
{
    QString username = qgetenv("USER");
    if (username.isEmpty())
        username = qgetenv("USERNAME");

    QVariantMap request;
    request["type"] = MessageLogin;
    request["username"] = username;
    request["method"] = LoginUser;

    Engine::write(request);
}

void LoginPage::loginByPassword()
{
    QString username = mUsernameEdit->text();
    QString password = mPasswordEdit->text();

    QVariantMap request;
    request["type"] = MessageLogin;
    request["username"] = username;
    request["password"] = password;
    request["method"] = LoginPassword;

    Engine::write(request);
    enableForm(false);
}

void LoginPage::onEngineConnected()
{
    loginByUser();
}

void LoginPage::onEngineDisconnected()
{
    enableForm(false);
}

void LoginPage::onEngineGotData(const QVariantMap &data)
{
    int type = data["type"].toInt();
    if (type == MessageLogin) {
        int method = data["method"].toInt();
        if (!data["online"].toBool()) {
            if (method == LoginPassword)
                QMessageBox::warning(this, "Login", "Username atau Password tidak sesuai!");

            enableForm(true);
            clearForm();
        }
    }
    else if (type == MessageLogout)
        enableForm(true);
}
