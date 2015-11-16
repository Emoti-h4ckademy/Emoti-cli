#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "network.h"
#include <QDate>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    cam(),
    camMutex(),
    sampleRate(MainWindow::SAMPLERATEDEFAULTs)
{
    this->ui->setupUi(this);

    //Camera
    this->cameraList_Setup();

    this->connect(this->ui->comboBoxCam, SIGNAL(currentIndexChanged(int)), this, SLOT (cameraChange()));
    this->connect(this->ui->checkBoxCamBlock, SIGNAL(clicked(bool)), this, SLOT (cameraChange()));
    this->connect(this->ui->checkBoxCamBuffer, SIGNAL(clicked(bool)), this, SLOT (cameraChange()));

    this->connect(this->ui->buttonUpdateCam, SIGNAL(released()), this, SLOT(cameraList_Setup()));


    //Send
    this->connect(this->ui->buttonSend, SIGNAL(clicked(bool)), this, SLOT(getImageAndSend()));
    this->ui->spinBoxSampleRate->setValue(this->sampleRate);
    this->ui->spinBoxSampleRate->setMaximum(MainWindow::SAMPLERATEMAXIMUMs);
    this->ui->spinBoxSampleRate->setMinimum(MainWindow::SAMPLERATEMINIMUMs);
    this->connect(this->ui->spinBoxSampleRate, SIGNAL(valueChanged(int)), this, SLOT(sampleRateChange()));




}

MainWindow::~MainWindow()
{
    delete ui;
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


void MainWindow::getImageAndSend()
{
    QString name = qgetenv("USER");
    if (name.isEmpty())
        name = qgetenv("USERNAME");

    std::shared_ptr<CamImage> img;

    img = cam.captureImageSync("png");

    if (img == nullptr)
    {
        qDebug() << Q_FUNC_INFO << "No image captured. Nothing is sent";
        return;
    }

    QDateTime time = QDateTime::currentDateTime();

    QString server = this->ui->textServer->toPlainText();

    Network net(server);
    net.sendImage(img, name, time.toString());
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

    this->sampleRate = newValue;

    this->getImageAndSend();
}
