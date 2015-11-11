#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "network.h"
#include <QDate>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    cam(),
    camListPosition(0)
{
    this->ui->setupUi(this);

    //Camera
    this->cameraList_Setup();
    this->connect(this->ui->camListBox, SIGNAL(currentIndexChanged(int)), this, SLOT(cameraList_Change()));

    //Button
    this->connect(this->ui->buttonSend, SIGNAL(clicked(bool)), this, SLOT(getImageAndSend()));

    //Text
    this->ui->textServer->setText("Url servidor");


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::cameraList_Setup()
{
    this->camList = QCameraInfo::availableCameras();

    for (QCameraInfo &cameraInfo : camList)
    {
        this->ui->camListBox->addItem(cameraInfo.deviceName());
        qDebug() << Q_FUNC_INFO << "Detected device: " << cameraInfo.deviceName();
    }

    if (!this->camList.isEmpty())
    {
        this->cameraList_Change();
    }

}

void MainWindow::cameraList_Change()
{
    int ind = this->ui->camListBox->currentIndex();

    if (ind >= this->camList.size())
    {
        qDebug() << Q_FUNC_INFO << "Invalid index";
        return;
    }

    QCameraInfo qi = this->camList.at(ind);
    this->cam.setup(qi); //TODO handle errors
}

void MainWindow::getImageAndSend()
{
    QString name = qgetenv("USER");
    if (name.isEmpty())
        name = qgetenv("USERNAME");
    auto img = cam.captureImageSync("PNG");

    QDateTime time = QDateTime::currentDateTime();

    QString server = this->ui->textServer->toPlainText();

    Network net(server);
    net.sendImage(img, name, time.toString());
}
