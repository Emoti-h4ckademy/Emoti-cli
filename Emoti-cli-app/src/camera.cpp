#include "camera.h"
#include <QDebug>
#include <QCoreApplication>

Camera::Camera()
    :cam(nullptr), camImageCapture(nullptr), deviceLocked(DEVICE_FREE), camMutex()
{
}


Camera::~Camera()
{
    if (camImageCapture) delete camImageCapture;
}


int Camera::setup(QCameraInfo& _device, lockStatus _deviceLocked)
{
    //setDevice already starts the camera
    qDebug() << Q_FUNC_INFO << "Setting up device: " << _device.deviceName() << "(" << _device.description() << "), " << (_deviceLocked == DEVICE_LOCKED ? "LOCKED":"FREE");
    return (this->setDevice(_device) || this->setDeviceMode(_deviceLocked));
}

int Camera::setDevice(QCameraInfo &_device)
{
    if (_device.isNull())
    {
        qDebug() << Q_FUNC_INFO << "Invalid device";
        return 1;
    }

    std::shared_ptr<QCamera> newCam = std::make_shared<QCamera> (new (std::nothrow) QCamera (_device));

    if (!newCam.get())
    {
        qDebug() << Q_FUNC_INFO << "Allocation error";
        return 1;
    }

    QCameraImageCapture* newImageCapture = new (std::nothrow) QCameraImageCapture(newCam.get());

    if (!newImageCapture)
    {
        qDebug() << Q_FUNC_INFO << "Allocation error";
        return 1;
    }

    if (!newImageCapture->isCaptureDestinationSupported(QCameraImageCapture::CaptureToFile))
    {
        qDebug() << Q_FUNC_INFO << "Cannot capture to memory buffer";
        delete newImageCapture;
        return 1;
    }

    //Everything is OK, swap the new camera and discard the old one
    this->cam = newCam;
    if (this->camImageCapture) delete camImageCapture;
    this->camImageCapture = newImageCapture;

    this->cam->setCaptureMode(QCamera::CaptureStillImage);
    this->camImageCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);

    return 0;

}

int Camera::setDeviceMode(Camera::lockStatus _deviceLocked)
{
    if ((this->deviceLocked == DEVICE_LOCKED) && (this->cam)) this->cam->stop();

    this->deviceLocked = _deviceLocked;

    return this->start(true);
}

std::shared_ptr<CamImage> Camera::captureImageSync(const char *_format)
{
    if (!this->camMutex.tryLock())
    {
        qDebug() << Q_FUNC_INFO << "Already capturing an image";
        return nullptr;
    }


    std::shared_ptr<CamImage> capturedImage;

    //Try to start the camera (if needed)
    if (this->start(false) && (!this->camImageCapture->isReadyForCapture()))
    {
        qDebug() << Q_FUNC_INFO << "Not ready for capture --- Camera state" << this->cam->state();
        this->camMutex.unlock();
        return nullptr;
    }

    //Signal capture
    QObject holder;
    QEventLoop eventLoop (&holder);
    std::vector <QMetaObject::Connection> myEvents;

    myEvents.push_back(holder.connect(this->camImageCapture, &QCameraImageCapture::imageAvailable,
                     &eventLoop, &QEventLoop::quit));
    myEvents.push_back(holder.connect(this->camImageCapture, SIGNAL(error(int,QCameraImageCapture::Error,QString)),
                     &eventLoop, SLOT(quit())));
    myEvents.push_back(holder.connect(this->camImageCapture, &QCameraImageCapture::readyForCaptureChanged,
                     &eventLoop, &QEventLoop::quit));
    //Bind the signal imageAvailable again but to a different function to save the image
    myEvents.push_back(holder.connect(this->camImageCapture, &QCameraImageCapture::imageAvailable,
                     [&capturedImage, &_format](int _id, const QVideoFrame &_image)
    {
        qDebug() << Q_FUNC_INFO << "ImageCaptured" << _id;
        capturedImage = qVideoFrame2CamImage (const_cast<QVideoFrame&> (_image), _format);
    }));

    //Capture
    this->cam->searchAndLock();
    this->camImageCapture->capture();
    this->cam->unlock();
    eventLoop.exec(); // <<<<<<<<<<<<< Holds until a signal is received

    //Disconnect from all signals
    for (auto it : myEvents)
    {
        holder.disconnect(it);
    }

    if (this->camImageCapture->error() != QCameraImageCapture::NoError)
    {
        qDebug() << Q_FUNC_INFO << this->camImageCapture->errorString()  << this->cam->status();
        capturedImage = nullptr;
    }

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
        if (this->deviceLocked == DEVICE_LOCKED) return this->startSync();
    }
    else
    {
        if (this->deviceLocked == DEVICE_FREE) return this->startSync();
    }

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
    myEvents.push_back(holder.connect(this->cam.get(), SIGNAL(error(QCamera::Error)),
                                      &eventLoop, SLOT(quit())));

    //Capture
    this->cam->start();
    eventLoop.exec(); //We wait until the camera has changed stated or an error has been received

    //Disconnect from all signals
    for (auto it : myEvents)
    {
        holder.disconnect(it);
    }

    if (this->cam->error() != QCamera::NoError)
    {
        qDebug() << Q_FUNC_INFO << this->cam->errorString();
        return false;
    }

    qDebug() << Q_FUNC_INFO << "Exit. Camera ready = " << this->camImageCapture->isReadyForCapture();
    return !this->camImageCapture->isReadyForCapture();
}
