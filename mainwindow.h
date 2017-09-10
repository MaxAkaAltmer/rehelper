#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:

    void makeTranslationFile(QString path);
    void parceNetLists(QString path);
};

#endif // MAINWINDOW_H
