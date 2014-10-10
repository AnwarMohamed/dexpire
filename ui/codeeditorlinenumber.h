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
