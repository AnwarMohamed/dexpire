#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QTreeView>
#include "treemodel.h"
#include "codeeditor.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("dexpire");
    QApplication::setApplicationVersion("0.1");

    //CodeEditor editor;
    //editor.setWindowTitle(QObject::tr("Code Editor Example"));
    //editor.show();

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
