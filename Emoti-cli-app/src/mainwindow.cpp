#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "network.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    cam(),
    camReady(false),
    camMutex(),
    sampleRateSec(MainWindow::SAMPLERATEDEFAULTs),
    lastSend(QDateTime::currentDateTime()),
    sendMutex(),
    sendTimer(),
    trayIcon(new (std::nothrow) QSystemTrayIcon()),
    trayIconMenu (new (std::nothrow) QMenu()),
    notificationsActive(true)
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

    //Server:
    this->ui->textServer->setText("http://localhost:3000/api/images");


    //Send
    this->connect(this->ui->buttonSend, SIGNAL(clicked(bool)), this, SLOT(sendImage()));
    this->ui->spinBoxSampleRate->setValue(this->sampleRateSec);
    this->ui->spinBoxSampleRate->setMaximum(MainWindow::SAMPLERATEMAXIMUMs);
    this->ui->spinBoxSampleRate->setMinimum(MainWindow::SAMPLERATEMINIMUMs);
    this->connect(this->ui->spinBoxSampleRate, SIGNAL(valueChanged(int)), this, SLOT(sampleRateChange()));

    //Timer
    this->sendTimer.setSingleShot(true);
    this->connect(&this->sendTimer, SIGNAL(timeout()), this, SLOT(sendImage()));

    //Tray
    this->trayIcon->setContextMenu(this->trayIconMenu);
    this->loadTray();

    //Notifications
    this->notificationsActive = this->trayIcon->supportsMessages();
    if (!this->notificationsActive)
    {
        this->ui->checkBoxNotifications->hide();
    }
    else
    {
        this->ui->checkBoxNotifications->setChecked(true);
        this->connect(this->ui->checkBoxNotifications, SIGNAL(clicked(bool)), this, SLOT(notificationsChange()));
    }

}

int MainWindow::loadTray()
{

    QAction *restoreAction = new QAction(QObject::tr("&Restore"), this);
    this->connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));
    this->trayIcon->contextMenu()->addAction(restoreAction);

    QAction *quitAction = new QAction(QObject::tr("&Quit"), this);
    this->connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    this->trayIcon->contextMenu()->addAction(quitAction);

    this->connect(this->trayIcon, &QSystemTrayIcon::activated, this,
                  [this](QSystemTrayIcon::ActivationReason _reason)
    {
        qDebug() << Q_FUNC_INFO << "Tray icon received action: " << _reason;
        if ((_reason != QSystemTrayIcon::Context))// || (_reason == QSystemTrayIcon::Trigger))
        {
            this->showNormal();
        }
    });

    this->trayIcon->setParent(this);
    this->trayIcon->setIcon(QIcon(":/resources/emoti-icon.png"));
    this->trayIcon->show();

    return 0;

}

MainWindow::~MainWindow()
{
    if (this->ui) delete ui;
    if (this->trayIcon) delete this->trayIcon;
    if (this->trayIconMenu) delete this->trayIconMenu;
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

    if ((!this->camReady) && (!this->camList.isEmpty()))
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
    this->sendTimer.stop();

    if (!this->cam.setup(qi,
                    block ? Camera::DEVICE_LOCKED : Camera::DEVICE_FREE,
                    file ? Camera::DESTINATION_FILE : Camera::DESTINATION_MEMORY))
    {
        this->camReady = true;
        qDebug() << Q_FUNC_INFO << "Next image will be sent in (ms): " << this->sampleRateSec * 1000;
        this->sendTimer.start(this->sampleRateSec * 1000);
    } else {
        this->camReady = false;
        this->trayIcon->showMessage("Emoti", "Could not start the camera");
    }

    this->camMutex.unlock();
}

void MainWindow::sendImage()
{
    if (this->camReady) {
        this->getImageAndSend(true);
    }
}


int MainWindow::getImageAndSend(bool ignoreRestriction)
{
    bool responseOK = false;
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
        responseOK = true;
        goto setTimerandExit;
   }

    img = cam.captureImageSync("jpg");

    if (img == nullptr)
    {
        qDebug() << Q_FUNC_INFO << "No image captured. Nothing is sent";
        this->trayIcon->showMessage("Emoti", "There seems to be a problem with the camera. Please check the select camera");
        responseOK = false;
        this->camReady = false;
        goto exitWithoutTimer;
    }


    responseOK = net.sendImage(img, this->username, actualTime.toString());
    if ((!responseOK) && (this->notificationsActive))
    {
        this->trayIcon->showMessage("Emoti", "There was an error sending the last image");
    }


setTimerandExit:
    long unsigned int timeout;
    if (responseOK)
    {
        this->lastSend = actualTime;
        timeout = this->sampleRateSec * 1000;
    }
    else
    {
        timeout = (MainWindow::SAMPLERATEFAILEDs < this->sampleRateSec ? MainWindow::SAMPLERATEFAILEDs : this->sampleRateSec ) * 1000;
    }

    this->sendTimer.stop();
    qDebug() << Q_FUNC_INFO << "Next image will be sent in (ms): " << timeout;
    this->sendTimer.start(timeout);

exitWithoutTimer:
    sendMutex.unlock();
    return (responseOK ? 0 : 1);


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

void MainWindow::notificationsChange()
{
    this->notificationsActive = this->ui->checkBoxNotifications->checkState();
}
