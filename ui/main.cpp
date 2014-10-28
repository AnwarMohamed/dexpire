/*
 *
 * Dexpire - main.cpp
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 *
 */

#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QTreeView>
#include "treemodel.h"
#include "codeeditor.h"
#include <QProgressDialog>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("dexpire");
    QApplication::setApplicationVersion("0.1");

    MainWindow mainWindow;
    mainWindow.showMaximized();

    return app.exec();
}
