#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"

#include "camera.h"

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

    Ui_MainWindow *ui;

    Camera cam;
    QList<QCameraInfo> camList;
    unsigned int camListPosition;
    void cameraList_Setup();

public slots:
    void cameraList_Change();
    void getImageAndSend();

};

#endif // MAINWINDOW_H
