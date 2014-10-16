#include "processthread.h"

ProcessThread::ProcessThread(cDexFile** dexFile, cDexDecompiler** dexDecompiler, char* dexPath, TreeModel** model, cApkFile** apkFile, QObject *parent) :
    QThread(parent)
{
    this->dexFile = dexFile;
    this->dexDecompiler = dexDecompiler;

    this->dexPath = new char[strlen(dexPath)+1];
    strncpy_s(this->dexPath, strlen(dexPath)+1, dexPath, strlen(dexPath));

    this->apkFile = apkFile;
    this->treeModel = model;
}

void ProcessThread::run()
{
    if (apkFile)
    {
        *apkFile = new cApkFile(dexPath);
        if ((*apkFile)->isReady)
        {
            for (unsigned int i=0; i<(*apkFile)->FilesCount; i++)
                if (strcmp((*apkFile)->Files[i].Name, "classes.dex") == 0)
                {
                    *dexFile = new cDexFile((*apkFile)->Files[i].Buffer, (*apkFile)->Files[i].Size);
                    if ((*dexFile)->isReady)
                    {
                        *dexDecompiler = new cDexDecompiler(*dexFile);
                        *treeModel = new TreeModel(*dexDecompiler);

                    }
                    break;
                }
        }
    }
    else
    {
        *dexFile = new cDexFile((char*)dexPath);
        if ((*dexFile)->isReady)
        {
            *dexDecompiler = new cDexDecompiler(*dexFile);
            *treeModel = new TreeModel(*dexDecompiler);
        }
    }
}
