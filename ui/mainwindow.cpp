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
#include <QTableView>
#include <QByteArray>
#include <cDexString.h>

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
    stringsTab = NULL;
    methodsTab = NULL;
    fieldsTab = NULL;
    typesTab = NULL;
    headerTab = NULL;
    protoTab = NULL;

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

    QMenu *menu = new QMenu("Tables");
    menu->setIcon(QIcon(":/icons/properties.gif"));
    menu->addAction(ui->actionFields_Table_2);
    menu->addAction(ui->actionStrings_Table_2);
    menu->addAction(ui->actionMethods_Table_2);
    menu->addAction(ui->actionTypes_Table_2);
    menu->addAction(ui->actionPrototypes_Table_2);
    toolbar->addAction(menu->menuAction());
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
    int index = tabOpened(QString("Fields Table"));
    if (index != -1)
        ui->tabWidget->setCurrentIndex(index);
    else
    {
        if (!fieldsTab)
        {
            fieldsTab = new QTableView(this);
            fieldsTab->setWordWrap(true);
            fieldsTab->setSelectionMode(QAbstractItemView::SingleSelection);
            fieldsTab->setEditTriggers(QAbstractItemView::NoEditTriggers);
            fieldsTab->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
            fieldsTab->verticalHeader()->setDefaultSectionSize(22);
            fieldsTab->setSelectionBehavior(QAbstractItemView::SelectRows);

            QObject::connect(fieldsTab, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(with_fieldsTab_doubleClicked(QModelIndex)));

            QStandardItemModel *model = new QStandardItemModel(dexFile->nFieldIDs, 3, this);
            model->setHorizontalHeaderItem(0, new QStandardItem(QString("Class Index")));
            model->setHorizontalHeaderItem(1, new QStandardItem(QString("Type Index")));
            model->setHorizontalHeaderItem(2, new QStandardItem(QString("Name")));

            for (unsigned int i=0; i<dexFile->nFieldIDs; i++)
            {
                model->setItem(i, 0, new QStandardItem(QString().sprintf("0x%04x", dexFile->DexFieldIds[i].ClassIndex)));
                model->setItem(i, 1, new QStandardItem(QString().sprintf("0x%04x", dexFile->DexFieldIds[i].TypeIdex)));
                model->setItem(i, 2, new QStandardItem(QString((char*)dexFile->StringItems[dexFile->DexFieldIds[i].StringIndex].Data)));
                model->setVerticalHeaderItem(i, new QStandardItem(QString().sprintf("0x%04x", i)));
            }

            fieldsTab->setModel(model);
            fieldsTab->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
        }

        ui->tabWidget->insertTab(0, fieldsTab, QString("Fields Table"));
        ui->tabWidget->setCurrentIndex(0);
    }
}

void MainWindow::with_fieldsTab_doubleClicked(const QModelIndex &index)
{
    switch(index.column())
    {
    case 0:
        on_actionTypes_Table_triggered();
        typesTab->selectRow(dexFile->DexFieldIds[index.row()].ClassIndex);
        break;

    case 1:
        on_actionTypes_Table_triggered();
        typesTab->selectRow(dexFile->DexFieldIds[index.row()].TypeIdex);
        break;

    case 2:
        on_actionStrings_Table_triggered();
        stringsTab->selectRow(dexFile->DexFieldIds[index.row()].StringIndex);
        break;
    }
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
}

void MainWindow::on_actionStrings_Table_triggered()
{
    int index = tabOpened(QString("Strings Table"));
    if (index != -1)
        ui->tabWidget->setCurrentIndex(index);
    else
    {
        if (!stringsTab)
        {
            stringsTab = new QTableView(this);
            stringsTab->setWordWrap(true);
            stringsTab->setSelectionMode(QAbstractItemView::SingleSelection);
            stringsTab->setEditTriggers(QAbstractItemView::NoEditTriggers);
            stringsTab->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
            stringsTab->verticalHeader()->setDefaultSectionSize(22);
            stringsTab->setSelectionBehavior(QAbstractItemView::SelectRows);

            QStandardItemModel *model = new QStandardItemModel(dexFile->nStringItems, 3, this);
            model->setHorizontalHeaderItem(0, new QStandardItem(QString("Offset")));
            model->setHorizontalHeaderItem(1, new QStandardItem(QString("Size")));
            model->setHorizontalHeaderItem(2, new QStandardItem(QString("String")));

            QStandardItem* item;
            for (unsigned int i=0; i<dexFile->nStringItems; i++)
            {
                model->setItem(i, 0, new QStandardItem(QString().sprintf("0x%08x", (unsigned long)dexFile->StringItems[i].Data - (unsigned long)dexFile->BaseAddress)));
                model->setItem(i, 1, new QStandardItem(QString().sprintf("0x%04x", dexFile->StringItems[i].StringSize)));
                item = new QStandardItem(QString((char*)dexFile->StringItems[i].Data).trimmed());
                item->setToolTip(item->text());
                model->setItem(i, 2, item);
                model->setVerticalHeaderItem(i, new QStandardItem(QString().sprintf("0x%04x", i)));
            }

            stringsTab->setModel(model);
            stringsTab->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
        }

        ui->tabWidget->insertTab(0, stringsTab, QString("Strings Table"));
        ui->tabWidget->setCurrentIndex(0);
    }
}

int MainWindow::tabOpened(QString& name)
{
   for (int i=0; i<ui->tabWidget->count(); i++)
       if (ui->tabWidget->tabText(i) == name)
           return i;
   return -1;
}

void MainWindow::on_actionHeader_triggered()
{
    int index = tabOpened(QString("Dex Header"));
    if (index != -1)
        ui->tabWidget->setCurrentIndex(index);
    else
    {
        if (!headerTab)
        {
            headerTab = new QTableView(this);
            headerTab->setWordWrap(true);
            headerTab->setEditTriggers(QAbstractItemView::NoEditTriggers);
            headerTab->setSelectionMode(QAbstractItemView::SingleSelection);
            headerTab->verticalHeader()->hide();
            headerTab->verticalHeader()->setDefaultSectionSize(22);
            headerTab->setSelectionBehavior(QAbstractItemView::SelectRows);

            QStandardItemModel *model = new QStandardItemModel(23, 2, this);
            model->setHorizontalHeaderItem(0, new QStandardItem(QString("Name")));
            model->setHorizontalHeaderItem(1, new QStandardItem(QString("Value")));

            model->setItem(0, 0, new QStandardItem(QString("Magic")));
            model->setItem(0, 1, new QStandardItem(QString((char*)dexFile->DexHeader->Magic)));

            model->setItem(1, 0, new QStandardItem(QString("Checksum")));
            model->setItem(1, 1, new QStandardItem(QString().sprintf("0x%08x", dexFile->DexHeader->Checksum)));

            model->setItem(2, 0, new QStandardItem(QString("Signature")));
            model->setItem(2, 1, new QStandardItem(QString(QByteArray::fromRawData((char*)dexFile->DexHeader->Signature, 20).toHex())));

            model->setItem(3, 0, new QStandardItem(QString("FileSize")));
            model->setItem(3, 1, new QStandardItem(QString().sprintf("0x%08x", dexFile->DexHeader->FileSize)));

            model->setItem(4, 0, new QStandardItem(QString("HeaderSize")));
            model->setItem(4, 1, new QStandardItem(QString().sprintf("0x%08x", dexFile->DexHeader->HeaderSize)));

            model->setItem(5, 0, new QStandardItem(QString("EndianTag")));
            model->setItem(5, 1, new QStandardItem(QString().sprintf("0x%08x", dexFile->DexHeader->EndianTag)));

            model->setItem(6, 0, new QStandardItem(QString("LinkSize")));
            model->setItem(6, 1, new QStandardItem(QString().sprintf("0x%08x", dexFile->DexHeader->LinkSize)));

            model->setItem(7, 0, new QStandardItem(QString("LinkOffset")));
            model->setItem(7, 1, new QStandardItem(QString().sprintf("0x%08x", dexFile->DexHeader->LinkOff)));

            model->setItem(8, 0, new QStandardItem(QString("MapOffset")));
            model->setItem(8, 1, new QStandardItem(QString().sprintf("0x%08x", dexFile->DexHeader->MapOff)));

            model->setItem(9, 0, new QStandardItem(QString("StringIdsSize")));
            model->setItem(9, 1, new QStandardItem(QString().sprintf("0x%08x", dexFile->DexHeader->StringIdsSize)));

            model->setItem(10, 0, new QStandardItem(QString("StringIdsOffset")));
            model->setItem(10, 1, new QStandardItem(QString().sprintf("0x%08x", dexFile->DexHeader->StringIdsOff)));

            model->setItem(11, 0, new QStandardItem(QString("TypeIdsSize")));
            model->setItem(11, 1, new QStandardItem(QString().sprintf("0x%08x", dexFile->DexHeader->TypeIdsSize)));

            model->setItem(12, 0, new QStandardItem(QString("TypeIdsOffset")));
            model->setItem(12, 1, new QStandardItem(QString().sprintf("0x%08x", dexFile->DexHeader->TypeIdsOff)));

            model->setItem(13, 0, new QStandardItem(QString("ProtoIdsSize")));
            model->setItem(13, 1, new QStandardItem(QString().sprintf("0x%08x", dexFile->DexHeader->ProtoIdsSize)));

            model->setItem(14, 0, new QStandardItem(QString("ProtoIdsOffset")));
            model->setItem(14, 1, new QStandardItem(QString().sprintf("0x%08x", dexFile->DexHeader->ProtoIdsOff)));

            model->setItem(15, 0, new QStandardItem(QString("FieldIdsSize")));
            model->setItem(15, 1, new QStandardItem(QString().sprintf("0x%08x", dexFile->DexHeader->FieldIdsSize)));

            model->setItem(16, 0, new QStandardItem(QString("FieldIdsOffset")));
            model->setItem(16, 1, new QStandardItem(QString().sprintf("0x%08x", dexFile->DexHeader->FieldIdsOff)));

            model->setItem(17, 0, new QStandardItem(QString("MethodIdsSize")));
            model->setItem(17, 1, new QStandardItem(QString().sprintf("0x%08x", dexFile->DexHeader->MethodIdsSize)));

            model->setItem(18, 0, new QStandardItem(QString("MethodIdsOffset")));
            model->setItem(18, 1, new QStandardItem(QString().sprintf("0x%08x", dexFile->DexHeader->MethodIdsOff)));

            model->setItem(19, 0, new QStandardItem(QString("ClassDefsSize")));
            model->setItem(19, 1, new QStandardItem(QString().sprintf("0x%08x", dexFile->DexHeader->ClassDefsSize)));

            model->setItem(20, 0, new QStandardItem(QString("ClassDefsOffset")));
            model->setItem(20, 1, new QStandardItem(QString().sprintf("0x%08x", dexFile->DexHeader->ClassDefsOff)));

            model->setItem(21, 0, new QStandardItem(QString("DataSize")));
            model->setItem(21, 1, new QStandardItem(QString().sprintf("0x%08x", dexFile->DexHeader->DataSize)));

            model->setItem(22, 0, new QStandardItem(QString("DataOffset")));
            model->setItem(22, 1, new QStandardItem(QString().sprintf("0x%08x", dexFile->DexHeader->DataOff)));


            headerTab->setModel(model);
            headerTab->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        }

        ui->tabWidget->insertTab(0, headerTab, QString("Dex Header"));
        ui->tabWidget->setCurrentIndex(0);
    }
}

void MainWindow::on_actionTypes_Table_triggered()
{
    int index = tabOpened(QString("Types Table"));
    if (index != -1)
        ui->tabWidget->setCurrentIndex(index);
    else
    {
        if (!typesTab)
        {
            typesTab = new QTableView(this);
            typesTab->setWordWrap(true);
            typesTab->setSelectionMode(QAbstractItemView::SingleSelection);
            typesTab->setEditTriggers(QAbstractItemView::NoEditTriggers);
            typesTab->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
            typesTab->verticalHeader()->setDefaultSectionSize(22);
            typesTab->setSelectionBehavior(QAbstractItemView::SelectRows);

            QStandardItemModel *model = new QStandardItemModel(dexFile->nTypeIDs, 2, this);
            model->setHorizontalHeaderItem(0, new QStandardItem(QString("Name")));
            model->setHorizontalHeaderItem(1, new QStandardItem(QString("Description")));

            for (unsigned int i=0; i<dexFile->nTypeIDs; i++)
            {
                model->setItem(i, 0, new QStandardItem(QString((char*)dexFile->StringItems[dexFile->DexTypeIds[i].StringIndex].Data)));
                model->setItem(i, 1, new QStandardItem(QString(
                    cDexString::GetTypeDescription((char*)dexFile->StringItems[dexFile->DexTypeIds[i].StringIndex].Data))));
                model->setVerticalHeaderItem(i, new QStandardItem(QString().sprintf("0x%04x", i)));
            }

            typesTab->setModel(model);
            typesTab->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        }

        ui->tabWidget->insertTab(0, typesTab, QString("Types Table"));
        ui->tabWidget->setCurrentIndex(0);
    }
}

void MainWindow::on_actionPrototypes_Table_triggered()
{
    int index = tabOpened(QString("Prototypes Table"));
    if (index != -1)
        ui->tabWidget->setCurrentIndex(index);
    else
    {
        if (!protoTab)
        {
            protoTab = new QTableView(this);
            protoTab->setWordWrap(true);
            protoTab->setSelectionMode(QAbstractItemView::SingleSelection);
            protoTab->setEditTriggers(QAbstractItemView::NoEditTriggers);
            protoTab->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
            protoTab->verticalHeader()->setDefaultSectionSize(22);
            protoTab->setSelectionBehavior(QAbstractItemView::SelectRows);

            QObject::connect(protoTab, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(with_protoTab_doubleClicked(QModelIndex)));

            QStandardItemModel *model = new QStandardItemModel(dexFile->nPrototypeIDs, 3, this);
            model->setHorizontalHeaderItem(0, new QStandardItem(QString("Shorty Index")));
            model->setHorizontalHeaderItem(1, new QStandardItem(QString("ReturnType Index")));
            model->setHorizontalHeaderItem(2, new QStandardItem(QString("Parameters Offset")));

            for (unsigned int i=0; i<dexFile->nPrototypeIDs; i++)
            {
                model->setItem(i, 0, new QStandardItem(QString().sprintf("0x%08x", dexFile->DexProtoIds[i].StringIndex)));
                model->setItem(i, 1, new QStandardItem(QString().sprintf("0x%08x", dexFile->DexProtoIds[i].ReturnTypeIdx)));
                model->setItem(i, 2, new QStandardItem(QString().sprintf("0x%08x", dexFile->DexProtoIds[i].ParametersOff)));
                model->setVerticalHeaderItem(i, new QStandardItem(QString().sprintf("0x%04x", i)));
            }

            protoTab->setModel(model);
            protoTab->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
        }

        ui->tabWidget->insertTab(0, protoTab, QString("Prototypes Table"));
        ui->tabWidget->setCurrentIndex(0);
    }
}

void MainWindow::with_protoTab_doubleClicked(const QModelIndex &index)
{
    switch(index.column())
    {
    case 0:
        on_actionStrings_Table_triggered();
        stringsTab->selectRow(dexFile->DexProtoIds[index.row()].StringIndex);
        break;

    case 1:
        on_actionTypes_Table_triggered();
        typesTab->selectRow(dexFile->DexProtoIds[index.row()].ReturnTypeIdx);
        break;

    case 2:
        //on_actionStrings_Table_triggered();
        //stringsTab->selectRow(dexFile->DexFieldIds[index.row()].StringIndex);
        break;
    }
}
void MainWindow::with_methodsTab_doubleClicked(const QModelIndex &index)
{
    switch(index.column())
    {
    case 0:
        on_actionTypes_Table_triggered();
        typesTab->selectRow(dexFile->DexMethodIds[index.row()].ClassIndex);
        break;

    case 1:
        on_actionPrototypes_Table_triggered();
        protoTab->selectRow(dexFile->DexMethodIds[index.row()].PrototypeIndex);
        break;

    case 2:
        on_actionStrings_Table_triggered();
        stringsTab->selectRow(dexFile->DexMethodIds[index.row()].StringIndex);
        break;
    }
}
void MainWindow::on_actionMethods_Table_triggered()
{
    int index = tabOpened(QString("Methods Table"));
    if (index != -1)
        ui->tabWidget->setCurrentIndex(index);
    else
    {
        if (!methodsTab)
        {
            methodsTab = new QTableView(this);
            methodsTab->setWordWrap(true);
            methodsTab->setSelectionMode(QAbstractItemView::SingleSelection);
            methodsTab->setEditTriggers(QAbstractItemView::NoEditTriggers);
            methodsTab->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
            methodsTab->verticalHeader()->setDefaultSectionSize(22);
            methodsTab->setSelectionBehavior(QAbstractItemView::SelectRows);

            QObject::connect(methodsTab, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(with_methodsTab_doubleClicked(QModelIndex)));

            QStandardItemModel *model = new QStandardItemModel(dexFile->nPrototypeIDs, 3, this);
            model->setHorizontalHeaderItem(0, new QStandardItem(QString("Class Index")));
            model->setHorizontalHeaderItem(1, new QStandardItem(QString("Prototype Index")));
            model->setHorizontalHeaderItem(2, new QStandardItem(QString("Name")));

            for (unsigned int i=0; i<dexFile->nPrototypeIDs; i++)
            {
                model->setItem(i, 0, new QStandardItem(QString().sprintf("0x%04x", dexFile->DexMethodIds[i].ClassIndex)));
                model->setItem(i, 1, new QStandardItem(QString().sprintf("0x%04x", dexFile->DexMethodIds[i].PrototypeIndex)));
                model->setItem(i, 2, new QStandardItem(QString((char*)dexFile->StringItems[dexFile->DexMethodIds[i].StringIndex].Data)));
                model->setVerticalHeaderItem(i, new QStandardItem(QString().sprintf("0x%04x", i)));
            }

            methodsTab->setModel(model);
            methodsTab->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
        }

        ui->tabWidget->insertTab(0, methodsTab, QString("Methods Table"));
        ui->tabWidget->setCurrentIndex(0);
    }
}
