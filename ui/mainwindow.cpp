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
#include "treemodel.h"
#include <QDirModel>
#include <cDexDecompiler.h>
#include <QLabel>
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    uiCenterScreen();
    uiSetupToolbar();
    uiSetupSplitter();
    uiSetupWorkspace();

    resizeEvent(NULL);

    dexFile = NULL;
    dexDecompiler = NULL;

    dexFile = new cDexFile("H:/Projects/dexpire/test/classes.dex");
    //delete dexFile;
    //dexFile = NULL;
    prepareDexWorkspace();
}

void MainWindow::uiSetupWorkspace()
{
    treeModel = NULL;

    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->treeView->setSelectionBehavior(QAbstractItemView::SelectItems);
}

void MainWindow::uiSetupSplitter()
{
    QList<int>* sizesList = new QList<int>();
    sizesList->append(180);
    sizesList->append(1);
    ui->splitter->setSizes(*sizesList);
    delete sizesList;

    ui->splitter->setStretchFactor(0, 0);
    ui->splitter->setStretchFactor(1, 1);
    ui->splitter->setCollapsible(0, false);
    ui->splitter->setCollapsible(1, false);
}

void MainWindow::loadFileDialog()
{
    QString Filename = QFileDialog::getOpenFileName(
                this, "Open File","classes.dex", "DEX File (*.dex);;APK File (*.apk)");
    if (!Filename.isEmpty())
    {
        if (Filename.endsWith(".dex"))
        {
            if (dexFile)
            {
                delete dexFile;
                dexFile = NULL;
            }

            dexFile = new cDexFile(Filename.toLocal8Bit().data());
            if (!dexFile->isReady)
                QMessageBox::critical(this, "Error", "Unable to process the specified file", QMessageBox::Ok);
            else
            {
                cleanCurrentWorkspace();
                prepareDexWorkspace();
            }
        }
    }
}

void MainWindow::cleanCurrentWorkspace()
{
    if (treeModel)
    {
        delete treeModel;
        treeModel = NULL;
    }
}



void MainWindow::prepareDexWorkspace()
{
    if (dexDecompiler)
        delete dexDecompiler;

    dexDecompiler = new cDexDecompiler(dexFile);
    treeModel = new TreeModel(dexDecompiler);
    ui->treeView->setModel(treeModel);
    ui->treeView->expandToDepth(0);

    ui->statusBar->showMessage(QString(dexFile->Filename).append(" loaded successfully."));
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
    connect(button ,SIGNAL(clicked()), this, SLOT(on_actionOpen_triggered()));
    toolbar->addWidget(button);

    button = new QToolButton;
    button->setIcon(QIcon(":/icons/document-save.png"));
    button->setToolTip("Save Class");
    button->setText(button->toolTip());
    button->setStatusTip(button->toolTip());
    connect(button ,SIGNAL(clicked()), this, SLOT(on_actionSave_Class_triggered()));
    toolbar->addWidget(button);

    button = new QToolButton;
    button->setIcon(QIcon(":/icons/document-save-as.png"));
    button->setToolTip("Save All");
    button->setText(button->toolTip());
    button->setStatusTip(button->toolTip());
    connect(button ,SIGNAL(clicked()), this, SLOT(on_actionSave_All_triggered()));
    toolbar->addWidget(button);
}

void MainWindow::resizeEvent(QResizeEvent*)
{
    ui->splitter->resize(
                geometry().width() - 10,
                geometry().height() - ui->mainToolBar->geometry().height() - 50);
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
    if (dexFile)
        delete dexFile;

    if (dexDecompiler)
        delete dexDecompiler;
}

void MainWindow::on_actionFields_Table_triggered()
{

}

void MainWindow::on_actionOpen_triggered()
{
    loadFileDialog();
}

void MainWindow::on_actionSave_Class_triggered()
{

}

void MainWindow::on_actionSave_All_triggered()
{

}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    ui->tabWidget->removeTab(index);
}

void MainWindow::on_tabWidget_currentChanged(int index)
{

}

void MainWindow::on_actionDex_Disassembly_triggered()
{

}

void MainWindow::on_treeView_clicked(const QModelIndex &index)
{
    TreeItem* item = ((TreeModel*)(ui->treeView->model()))->getChild(index);
    std::cout << item->getText().toString().toStdString() << std::endl;
}
