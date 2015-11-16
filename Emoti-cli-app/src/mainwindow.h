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
    const int TRYLOCKms = 100;
    const int SAMPLERATEDEFAULTs = 300;
    const int SAMPLERATEMINIMUMs = 10;
    const int SAMPLERATEMAXIMUMs = 3600;

    Ui_MainWindow *ui;
    Camera cam;
    QMutex camMutex;
    QList<QCameraInfo> camList;
    int sampleRate;


public slots:
    void cameraList_Setup();
    void cameraChange();
    void getImageAndSend();
    void sampleRateChange();

};

#endif // MAINWINDOW_H
