/*
 *
 * Dexpire - codeeditorlinenumber.cpp
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 *
 */

#include "codeeditorlinenumber.h"
#include <QFont>

CodeEditorLineNumber::CodeEditorLineNumber(CodeEditor *editor) :
    QWidget(editor)
{
    codeEditor = editor;

    QFont font("Consolas");
    font.setStyleHint(QFont::Monospace);
    font.setPointSize(10);
    setFont(font);
}

QSize CodeEditorLineNumber::sizeHint() const
{
    return QSize(codeEditor->lineNumberAreaWidth(), 0);
}

void CodeEditorLineNumber::paintEvent(QPaintEvent *event)
{
    codeEditor->lineNumberAreaPaintEvent(event);
}
