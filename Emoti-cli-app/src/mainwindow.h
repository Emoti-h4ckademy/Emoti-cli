#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDate>
#include <QTimer>

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
    const int SAMPLERATEFAILEDs = SAMPLERATEMINIMUMs;

    Ui_MainWindow *ui;
    Camera cam;
    QMutex camMutex;
    QList<QCameraInfo> camList;
    QString username;

    int sampleRateSec;
    QDateTime lastSend;
    QMutex sendMutex;
    QTimer sendTimer;

    int getImageAndSend(bool ignoreRestriction);


public slots:
    void cameraList_Setup();
    void cameraChange();
    void sendImage();
    void sampleRateChange();

};

#endif // MAINWINDOW_H
