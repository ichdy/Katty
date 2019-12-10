#include "mainwindow.h"
#include <QApplication>
#include <QFontDatabase>

#include "engine.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFontDatabase::addApplicationFont(":/fonts/NotoSans-Regular.ttf");

    MainWindow w;
    w.show();

    return a.exec();
}
