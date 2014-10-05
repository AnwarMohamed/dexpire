#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("dexpire");
    QApplication::setApplicationVersion("0.1");

    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
