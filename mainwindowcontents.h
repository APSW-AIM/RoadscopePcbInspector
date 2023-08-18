#ifndef MAINWINDOWCONTENTS_H
#define MAINWINDOWCONTENTS_H

#include <QtWidgets/QWidget>

#include "ui_mainwindowcontents.h"

//namespace Ui {
//class MainWindowContents;
//}

class MainWindow;

class MainWindowContents : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindowContents(QWidget* parent = nullptr);
    ~MainWindowContents();

private:
    Ui::MainWindowContents* m_pUi = nullptr;

private:
    friend MainWindow;
};

#endif // MAINWINDOWCONTENTS_H
