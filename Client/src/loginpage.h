#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class LoginPage : public QWidget
{
    Q_OBJECT
public:
    explicit LoginPage(QWidget *parent = nullptr);

public slots:
    void enableForm(bool value);
    void clearForm();
    void loginByUser();
    void loginByPassword();

protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void onEngineConnected();
    void onEngineDisconnected();
    void onEngineGotData(const QVariantMap &data);

private:
    QLabel *mIconLabel;
    QLineEdit *mUsernameEdit;
    QLineEdit *mPasswordEdit;
    QPushButton *mLoginButton;
};

#endif // LOGINPAGE_H
