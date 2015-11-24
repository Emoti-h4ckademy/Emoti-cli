#include "camera.h"
#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QTemporaryFile>
#include <QTimer>

Camera::Camera()
    :cam(nullptr),
      camImageCapture(nullptr),
      deviceLocked(DEVICE_FREE),
      camMutex(),
      camDestination(DESTINATION_MEMORY),
      fTemplate(QDir::tempPath() + "/emoti-XXXXXX.jpg")
{
}


Camera::~Camera()
{
    if (this->camImageCapture)
    {
        delete this->camImageCapture;
    }
    if (this->cam)
    {
        this->stop(true);
    }
}


int Camera::setup(QCameraInfo& _device, lockStatus _deviceLocked, imageDestination _tempDestination)
{
    if (!this->camMutex.tryLock())
    {
        qDebug() << Q_FUNC_INFO << "Camera is busy";
        return 1;
    }

#ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << Q_FUNC_INFO << "Setting up device: " << _device.deviceName() << "(" << _device.description() << "), "
             << (_deviceLocked == DEVICE_LOCKED ? "LOCKED":"FREE")
             << (_tempDestination == DESTINATION_FILE ? "FILE":"MEMORY");
#endif

    //setDevice already starts the camera
    bool result =  (this->setDevice(_device, _tempDestination) || this->setDeviceMode(_deviceLocked));

    this->camMutex.unlock();

    return result;
}


int Camera::setDevice(QCameraInfo &_device, imageDestination _tempDestination)
{
    if (_device.isNull())
    {
        qDebug() << Q_FUNC_INFO << "Invalid device";
        return 1;
    }

    std::shared_ptr<QCamera> newCam (new (std::nothrow) QCamera (_device));
    if (!newCam.get())
    {
        qDebug() << Q_FUNC_INFO << "Allocation error";
        return 1;
    }
    newCam->setCaptureMode(QCamera::CaptureStillImage);

    QCameraImageCapture* newImageCapture = new (std::nothrow) QCameraImageCapture(newCam.get());
    if (!newImageCapture)
    {
        qDebug() << Q_FUNC_INFO << "Allocation error";
        return 1;
    }

    if (_tempDestination == Camera::DESTINATION_MEMORY && newImageCapture->isCaptureDestinationSupported(QCameraImageCapture::CaptureToBuffer))
    {
        newImageCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
        this->camDestination = Camera::DESTINATION_MEMORY;
        qDebug() << Q_FUNC_INFO << "Buffering image in memory";
    }
    else
    {
        newImageCapture->setCaptureDestination(QCameraImageCapture::CaptureToFile);
        this->camDestination = Camera::DESTINATION_FILE;
        qDebug() << Q_FUNC_INFO << "Buffering image in a temporal file";
    }

    //Everything is OK, swap the new camera and discard the old one
    this->cam = newCam;
    if (this->camImageCapture) delete camImageCapture;
    this->camImageCapture = newImageCapture;

    return 0;
}


int Camera::setDeviceMode(Camera::lockStatus _deviceLocked)
{
    if ((this->deviceLocked == DEVICE_LOCKED) && (this->cam)) this->cam->stop();

    this->deviceLocked = _deviceLocked;

    return this->start(true);
}

std::shared_ptr<CamImage> Camera::captureImageSync_memory(const char *_format)
{
    std::shared_ptr<CamImage> capturedImage = nullptr;

    //Signal capture
    QObject holder;
    QEventLoop eventLoop (&holder);
    std::vector <QMetaObject::Connection> myEvents;

    myEvents.push_back(holder.connect(this->camImageCapture, &QCameraImageCapture::imageAvailable,
                     &eventLoop, &QEventLoop::quit));
    myEvents.push_back(holder.connect(this->camImageCapture, SIGNAL(error(int,QCameraImageCapture::Error,QString)),
                     &eventLoop, SLOT(quit())));

    //Capture image from memory
    myEvents.push_back(holder.connect(this->camImageCapture, &QCameraImageCapture::imageAvailable,
                     [&capturedImage, &_format](int _id, const QVideoFrame &_image)
    {
        qDebug() << Q_FUNC_INFO << "imageAvailable" << _id;
        capturedImage = qVideoFrame2CamImage (const_cast<QVideoFrame&> (_image), _format);
    }));

    this->cam->searchAndLock();
    this->camImageCapture->capture();
    this->cam->unlock();

    eventLoop.exec(); // <<<<<<<<<<<<< Holds until a signal is received

    //Disconnect from all signals
    for (auto it : myEvents)
    {
        holder.disconnect(it);
    }

#ifndef QT_NO_DEBUG_OUTPUT
    if (this->camImageCapture->error() != QCameraImageCapture::NoError)
    {
        qDebug() << Q_FUNC_INFO << this->camImageCapture->errorString()  << this->cam->status();
    }
#endif

    return capturedImage;
}

std::shared_ptr<CamImage> Camera::captureImageSync_file(const char *_format)
{
    std::shared_ptr<CamImage> capturedImage = nullptr;

    //Signal capture
    QObject holder;
    QEventLoop eventLoop (&holder);
    std::vector <QMetaObject::Connection> myEvents;

    myEvents.push_back(holder.connect(this->camImageCapture, &QCameraImageCapture::imageSaved,
                     &eventLoop, &QEventLoop::quit));
    myEvents.push_back(holder.connect(this->camImageCapture, SIGNAL(error(int,QCameraImageCapture::Error,QString)),
                     &eventLoop, SLOT(quit())));

    //Capture image from file
    myEvents.push_back(holder.connect(this->camImageCapture, &QCameraImageCapture::imageSaved,
                     [&capturedImage, &_format](int _id, const QString &_fileName)
    {
        qDebug() << Q_FUNC_INFO << "imageSaved" << _id << "Path: " << _fileName;
        capturedImage = jpgFile2CamImage(_fileName, _format);
    }));

    //Setup temp file --> path
    QTemporaryFile myfile (this->fTemplate);
    if (!myfile.open()) {
        qDebug() << Q_FUNC_INFO << "Cannot create a temp file. Template: " << fTemplate;
        return nullptr;
    }

    qDebug() << Q_FUNC_INFO << "TEMP. Filename: " << myfile.fileName() << " Autoremove: " << myfile.autoRemove();

    this->cam->searchAndLock();
    this->camImageCapture->capture(myfile.fileName());
    this->cam->unlock();

    eventLoop.exec(); // <<<<<<<<<<<<< Holds until a signal is received

    myfile.close();

    //Disconnect from all signals
    for (auto it : myEvents)
    {
        holder.disconnect(it);
    }

#ifndef QT_NO_DEBUG_OUTPUT
    if (this->camImageCapture->error() != QCameraImageCapture::NoError)
    {
        qDebug() << Q_FUNC_INFO << this->camImageCapture->errorString()  << this->cam->status();
    }
#endif

    return capturedImage;
}



std::shared_ptr<CamImage> Camera::captureImageSync(const char *_format)
{
    if (!this->camMutex.tryLock())
    {
        qDebug() << Q_FUNC_INFO << "Camera is busy";
        return nullptr;
    }

    //Try to start the camera (if needed)
    if (!this->start(false))
    {
        qDebug() << Q_FUNC_INFO << "Not ready for capture";

        this->camMutex.unlock();
        return nullptr;
    }

    std::shared_ptr<CamImage> capturedImage (this->camDestination == Camera::DESTINATION_MEMORY ?
                                                 captureImageSync_memory(_format):
                                                 captureImageSync_file(_format));

    //Stop the camera if needed
    this->stop(false);
    this->camMutex.unlock();

    return capturedImage;
}

int Camera::start(bool _firstTime)
{
    qDebug() << Q_FUNC_INFO << "called. First time: " << _firstTime;
    if (_firstTime)
    {
        //Firsttime && Device locked
        if (this->deviceLocked == DEVICE_LOCKED) return this->startSync();
    }
    else
    {
        //No firsttime && Device free
        if (this->deviceLocked == DEVICE_FREE) return this->startSync();
    }

    //The other two options (first time and device free, no first time and device locked)
    return 0;
}

int Camera::stop(bool _forceStop)
{
    qDebug() << Q_FUNC_INFO << "called";
    if (!this->cam)
    {
        qDebug() << Q_FUNC_INFO << "Not initialized";
        return 1;
    }

    if ((_forceStop) || (this->deviceLocked == DEVICE_FREE))
    {
        this->cam->stop();
    }

    return 0;
}


bool Camera::startSync()
{
    if (!this->cam)
    {
        qDebug() << Q_FUNC_INFO << "Not initialized";
        return false;
    }

    if (this->camImageCapture->isReadyForCapture())
    {
        qDebug() << Q_FUNC_INFO << "Camera ready. NOTHING IS DONE";
        return true;
    }

    //Signal capture
    QObject holder;
    QEventLoop eventLoop (&holder);
    std::vector <QMetaObject::Connection> myEvents;
    myEvents.push_back(holder.connect(this->camImageCapture, &QCameraImageCapture::readyForCaptureChanged,
                                      &eventLoop, &QEventLoop::quit));
    myEvents.push_back(holder.connect(this->camImageCapture, SIGNAL(error(int,QCameraImageCapture::Error,QString)),
                                      &eventLoop, SLOT(quit())));
    QTimer timer;
    myEvents.push_back(holder.connect(&timer, SIGNAL(timeout()),&eventLoop, SLOT(quit())));
    timer.start(this->TIMEOUTs * 1000);

    //Capture
    this->cam->start();
    eventLoop.exec(); //We wait until the camera has changed stated or an error has been received

    //Disconnect from all signals
    for (auto it : myEvents)
    {
        holder.disconnect(it);
    }


    if (!this->camImageCapture->isReadyForCapture())
    {
        qDebug() << Q_FUNC_INFO << "COULD NOT START WEBCAM: " + this->cam->errorString();
        return false;
    }

    return true;
}
