#include "mainwindow.h"
#include <QApplication>

#include "engine.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    Engine *engine = Engine::instance();
    engine->start();

    return a.exec();
}
