#include "codeeditorlinenumber.h"

CodeEditorLineNumber::CodeEditorLineNumber(CodeEditor *editor) :
    QWidget(editor)
{
    codeEditor = editor;
}

QSize CodeEditorLineNumber::sizeHint() const
{
    return QSize(codeEditor->lineNumberAreaWidth(), 0);
}

void CodeEditorLineNumber::paintEvent(QPaintEvent *event)
{
    codeEditor->lineNumberAreaPaintEvent(event);
}
