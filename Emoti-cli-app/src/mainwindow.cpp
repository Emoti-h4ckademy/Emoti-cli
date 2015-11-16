#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "network.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    cam(),
    camMutex(),
    sampleRateSec(MainWindow::SAMPLERATEDEFAULTs),
    lastSend(QDateTime::currentDateTime()),
    sendMutex(),
    sendTimer()
{
    this->username = qgetenv("USER");
    if (this->username.isEmpty())
    {
        this->username = qgetenv("USERNAME");
    }
    if (this->username.isEmpty())
    {
        qDebug() << Q_FUNC_INFO << "Cannot get username from system";
        this->username = "Emoti";
    }


    this->ui->setupUi(this);

    //Camera
    this->cameraList_Setup();

    this->connect(this->ui->comboBoxCam, SIGNAL(currentIndexChanged(int)), this, SLOT (cameraChange()));
    this->connect(this->ui->checkBoxCamBlock, SIGNAL(clicked(bool)), this, SLOT (cameraChange()));
    this->connect(this->ui->checkBoxCamBuffer, SIGNAL(clicked(bool)), this, SLOT (cameraChange()));

    this->connect(this->ui->buttonUpdateCam, SIGNAL(released()), this, SLOT(cameraList_Setup()));


    //Send
    this->connect(this->ui->buttonSend, SIGNAL(clicked(bool)), this, SLOT(sendImage()));
    this->ui->spinBoxSampleRate->setValue(this->sampleRateSec);
    this->ui->spinBoxSampleRate->setMaximum(MainWindow::SAMPLERATEMAXIMUMs);
    this->ui->spinBoxSampleRate->setMinimum(MainWindow::SAMPLERATEMINIMUMs);
    this->connect(this->ui->spinBoxSampleRate, SIGNAL(valueChanged(int)), this, SLOT(sampleRateChange()));

    //Timer
    this->sendTimer.setSingleShot(true);
    this->connect(&this->sendTimer, SIGNAL(timeout()), this, SLOT(sendImage()));
    this->sendTimer.start(this->sampleRateSec * 1000);


}

MainWindow::~MainWindow()
{
    if (this->ui)
    {
        delete ui;
    }
}

void MainWindow::cameraList_Setup()
{
    if (!this->camMutex.tryLock(MainWindow::TRYLOCKms))
    {
        qDebug() << Q_FUNC_INFO << "Already updating";
        return;
    }


    this->ui->comboBoxCam->clear();
    this->camList = QCameraInfo::availableCameras();

    for (QCameraInfo &cameraInfo : this->camList)
    {
        this->ui->comboBoxCam->addItem(cameraInfo.description());
        qDebug() << Q_FUNC_INFO << "Detected device: " << cameraInfo.deviceName() << "(" << cameraInfo.description() << ")";
    }

    this->camMutex.unlock();

    if (!this->camList.isEmpty())
    {
        this->cameraChange();
    }
}

void MainWindow::cameraChange()
{
    if (!this->camMutex.tryLock(MainWindow::TRYLOCKms))
    {
        qDebug() << Q_FUNC_INFO << "Already updating";
        return;
    }

    bool block = this->ui->checkBoxCamBlock->isChecked();
    bool file = this->ui->checkBoxCamBuffer->isChecked();

    int ind = this->ui->comboBoxCam->currentIndex();

    if ((ind < 0) || (ind >= this->camList.size()))
    {
        qDebug() << Q_FUNC_INFO << "Invalid index";
        return;
    }

    QCameraInfo qi = this->camList.at(ind);
    this->cam.setup(qi,
                    block ? Camera::DEVICE_LOCKED : Camera::DEVICE_FREE,
                    file ? Camera::DESTINATION_FILE : Camera::DESTINATION_MEMORY); //TODO handle errors

    this->camMutex.unlock();
}

void MainWindow::sendImage()
{
    this->getImageAndSend(true);
}


int MainWindow::getImageAndSend(bool ignoreRestriction)
{
    int neterror = 0;
    QDateTime nextSend = this->lastSend.addSecs(this->sampleRateSec);
    QDateTime actualTime = QDateTime::currentDateTime();
    std::shared_ptr<CamImage> img;
    QString server = this->ui->textServer->toPlainText();
    Network net (server);

    if (!this->sendMutex.tryLock(MainWindow::TRYLOCKms))
    {
        qDebug() << Q_FUNC_INFO << "Already sending";
        return 1;
    }

   if ((!ignoreRestriction) && (nextSend > actualTime))
   {
        qDebug() << Q_FUNC_INFO << "Too soon to send an image. Asked for: " << actualTime.toString() << ". Next configured: " << nextSend.toString();
        neterror = 1;
        goto setTimerandExit;
   }

    img = cam.captureImageSync("png");

    if (img == nullptr)
    {
        qDebug() << Q_FUNC_INFO << "No image captured. Nothing is sent";
        neterror = 1;
        goto setTimerandExit;
    }


    neterror = net.sendImage(img, this->username, actualTime.toString());


setTimerandExit:
    long unsigned int timeout;
    if (!neterror)
    {
        this->lastSend = actualTime;
        timeout = this->sampleRateSec * 1000;
    }
    else
    {
        timeout = nextSend.toMSecsSinceEpoch() - actualTime.toMSecsSinceEpoch();
    }

    this->sendTimer.stop();
    qDebug() << Q_FUNC_INFO << "Next image will be send in (ms): " << timeout;
    this->sendTimer.start(timeout);

    sendMutex.unlock();
    return neterror;


}

void MainWindow::sampleRateChange()
{
    int newValue = this->ui->spinBoxSampleRate->value();

    if (newValue < MainWindow::SAMPLERATEMINIMUMs)
    {
        qDebug() << Q_FUNC_INFO << "Sample rate too low: " << newValue << ". Expected > " << MainWindow::SAMPLERATEMINIMUMs;
        newValue = MainWindow::SAMPLERATEMINIMUMs;
    }

    if (newValue > MainWindow::SAMPLERATEMAXIMUMs)
    {
        qDebug() << Q_FUNC_INFO << "Sample rate too high: " << newValue << ". Expected < " << MainWindow::SAMPLERATEMAXIMUMs;
        newValue = MainWindow::SAMPLERATEMAXIMUMs;
    }

    this->sampleRateSec = newValue;



    this->getImageAndSend(false);
}
