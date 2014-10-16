#ifndef PROCESSTHREAD_H
#define PROCESSTHREAD_H

#include <QThread>
#include <cDexDecompiler.h>
#include <cDexFile.h>
#include <cApkFile.h>
#include "treemodel.h"

class ProcessThread : public QThread
{
    Q_OBJECT
public:
    explicit ProcessThread(cDexFile** dexFile, cDexDecompiler** dexDecompiler, char* dexPath, TreeModel** model, cApkFile** apkFile=0, QObject *parent = 0);

    void run();
signals:

public slots:

private:
    cDexFile** dexFile;
    cDexDecompiler** dexDecompiler;
    cApkFile** apkFile;
    char* dexPath;
    TreeModel** treeModel;
};

#endif // PROCESSTHREAD_H
