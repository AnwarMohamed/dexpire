#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QToolBar* toolbar;

    void uiCenterScreen();
    void uiSetupToolbar();

protected:
    void resizeEvent(QResizeEvent * event);
    //void closeEvent(QCloseEvent *event);

private slots:
    void menuOpen();
};

#endif // MAINWINDOW_H
