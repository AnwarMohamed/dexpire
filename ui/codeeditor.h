/*
 *
 * Dexpire - codeeditor.h
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 *
 */

#ifndef CODEEDITOR_H
#define CODEEDITOR_H
#include <QPlainTextEdit>
 #include <QObject>

 class QPaintEvent;
 class QResizeEvent;
 class QSize;
 class QWidget;

 class LineNumberArea;


 class CodeEditor : public QPlainTextEdit
 {
     Q_OBJECT

 public:
     CodeEditor(QWidget *parent = 0);

     void lineNumberAreaPaintEvent(QPaintEvent *event);
     int lineNumberAreaWidth();

 protected:
     void resizeEvent(QResizeEvent *event);

 private slots:
     void updateLineNumberAreaWidth(int newBlockCount);
     void highlightCurrentLine();
     void updateLineNumberArea(const QRect &, int);

 private:
     QWidget *lineNumber;
 };

#endif // CODEEDITOR_H
