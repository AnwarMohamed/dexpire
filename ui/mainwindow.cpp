#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDesktopWidget>
#include <QToolButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardItemModel>
#include <cDexFile.h>
#include <QSplitter>
#include <QTreeView>
#include <QTextEdit>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    uiCenterScreen();
    uiSetupToolbar();

    resizeEvent(NULL);

    QList<int>* sizesList = new QList<int>();
    sizesList->append(100);
    sizesList->append(400);
    ui->splitter->setSizes(*sizesList);

    ui->splitter->setCollapsible(0, false);
    ui->splitter->setCollapsible(1, false);
}

void MainWindow::menuOpen()
{
    QString filename = QFileDialog::getOpenFileName(
                this, "Open File","classes.dex", "DEX File (*.dex);;APK File (*.apk)");
    if (!filename.isEmpty())
    {

        //if (!processFilename())
        //    QMessageBox::critical(this, "Error", "Unable to process the specified file", QMessageBox::Ok);
        //else
        {

        }
    }
}

void MainWindow::uiSetupToolbar()
{
    toolbar = ui->mainToolBar;
    toolbar->setContextMenuPolicy(Qt::PreventContextMenu);

    QToolButton* button = new QToolButton;
    button->setIcon(QIcon(":/icons/document-open-folder.png"));
    button->setToolTip("Open");
    button->setText(button->toolTip());
    button->setStatusTip(button->toolTip());
    connect(button ,SIGNAL(clicked()), this, SLOT(menuOpen()));
    toolbar->addWidget(button);

    button = new QToolButton;
    button->setIcon(QIcon(":/icons/document-save.png"));
    button->setToolTip("Save");
    button->setText(button->toolTip());
    button->setStatusTip(button->toolTip());

    //connect(button ,SIGNAL(clicked()), this, SLOT(deleteMenu()));
    toolbar->addWidget(button);

    button = new QToolButton;
    button->setIcon(QIcon(":/icons/document-save-as.png"));
    button->setToolTip("Save As");
    button->setText(button->toolTip());
    button->setStatusTip(button->toolTip());

    //connect(button ,SIGNAL(clicked()), this, SLOT(searchMenu()));
    toolbar->addWidget(button);

}

void MainWindow::resizeEvent(QResizeEvent*)
{
    ui->splitter->resize(
                geometry().width() - 10,
                geometry().height() - ui->mainToolBar->geometry().height() - 50
                );

    /*ui->tableStructure->resize(
                ui->tabWidget->geometry().width() - 14,
                ui->tabWidget->geometry().height() - 40
                );*/
}

void MainWindow::uiCenterScreen()
{
    QRect frect = frameGeometry();
    frect.moveCenter(QDesktopWidget().availableGeometry().center());
    move(frect.topLeft());
}

MainWindow::~MainWindow()
{
    delete ui;
}
