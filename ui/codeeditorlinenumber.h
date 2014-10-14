/*
 *
 * Dexpire - codeeditorlinenumber.h
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 *
 */

#ifndef CODEEDITORLINENUMBER_H
#define CODEEDITORLINENUMBER_H

#include <QWidget>
#include "codeeditor.h"

class CodeEditorLineNumber : public QWidget
{
public:
    CodeEditorLineNumber(CodeEditor *editor);

    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *event);

private:
    CodeEditor *codeEditor;
};


#endif // CODEEDITORLINENUMBER_H
