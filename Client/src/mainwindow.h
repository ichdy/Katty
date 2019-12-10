#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class Stream;
class LoadingOverlay;
class LoginPage;
class MainPage;
class QStackedWidget;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void onEngineConnected();
    void onEngineDisconnected();
    void onEngineGotData(const QVariantMap &data);

private:
    LoadingOverlay *mLoading;
    QStackedWidget *mStack;
    LoginPage *mLoginPage;
    MainPage *mMainPage;
};

#endif // MAINWINDOW_H
