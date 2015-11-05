#include "camera.h"
#include <QDebug>
#include <QCoreApplication>


CamImage::CamImage()
    :data(nullptr), size(0)
{
}

size_t CamImage::getSize()
{
    return this->size;
}

std::shared_ptr<unsigned char> CamImage::getData()
{
    return this->data;
}

int CamImage::appendData(unsigned char *_data, size_t _size)
{
    size_t nsize = this->size + _size;

    if (this->data == nullptr)
    {
        //Create space and copy the data
        unsigned char *np = static_cast<unsigned char*> (malloc (_size * sizeof(unsigned char)));
        if (!np) goto failure;

        memcpy(np, _data, _size);

        this->data.reset(np,free);
        this->size = _size;
    } else {
        //Creates enough space for current + new data
        unsigned char *np = static_cast<unsigned char*> (malloc (nsize * sizeof(unsigned char)));
        if (!np) goto failure;

        memcpy(np, this->data.get(), this->size);
        memcpy(&np[this->size],_data, _size);

        this->data.reset(np,free);
        this->size = nsize;

    }

    return 0;

  failure:
    qDebug() << Q_FUNC_INFO << "Could not allocate memory";
    return 1;
}

Camera::Camera()
    :cam(nullptr), camImageCapture(nullptr), deviceLocked(DEVICE_FREE)
{
}

Camera::~Camera()
{
    if (camImageCapture) delete camImageCapture;
}


int Camera::setup(QCameraInfo& _device, lockStatus _deviceLocked)
{
    if (_device.isNull())
    {
        qDebug() << Q_FUNC_INFO << "Invalid device";
        return 1;
    }

    /**
      TODO: Separate into  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    int setDevice (QCameraInfo& _device);
    int setDeviceMode (lockStatus _deviceLocked);
    */

    //Check that we can allocate memory and there's no problem with the device
    auto oldCam = this->cam;
    this->cam = std::make_shared<QCamera> (new (std::nothrow) QCamera (_device));

    if ((cam == nullptr || this->cam->error() != QCamera::NoError))
    {
        qDebug() << Q_FUNC_INFO << this->cam->errorString();
        this->cam = oldCam;
        return 1;
    }

    //Same for CameraImageCapture
    auto oldImgCap = this->camImageCapture;
    this->camImageCapture = new (std::nothrow) QCameraImageCapture(cam.get());


    if ((camImageCapture == nullptr) || (this->camImageCapture->error() != QCameraImageCapture::NoError))
    {
        qDebug() << Q_FUNC_INFO << (camImageCapture == nullptr ? "Allocation" : this->camImageCapture->errorString());
        this->cam = oldCam;
        if (camImageCapture != nullptr) delete camImageCapture;
        this->camImageCapture = oldImgCap;

        return 1;
    }

    if (!this->camImageCapture->isCaptureDestinationSupported(QCameraImageCapture::CaptureToFile))
    {
        qDebug() << Q_FUNC_INFO << "Cannot capture to memory buffer";
        this->cam = oldCam;
        if (camImageCapture != nullptr) delete camImageCapture;
        this->camImageCapture = oldImgCap;

        return 1;
    }


    //SETUP the new objects
    this->cam->setCaptureMode(QCamera::CaptureStillImage);
    this->deviceCurrent = _device.deviceName();
    this->camImageCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
    this->deviceLocked = _deviceLocked;
    if (_deviceLocked == DEVICE_LOCK) this->cam->start();


    //Logs state changes
    QObject::connect((this->cam.get()), &QCamera::statusChanged,
                     [=] ()
    {
        qDebug() << Q_FUNC_INFO << "State changed" << this->cam->status();
    });

    return 0;
}

std::shared_ptr<CamImage> Camera::captureImageSync()
{
    if (this->cam == nullptr)
    {
        qDebug() << Q_FUNC_INFO << "Camera not created";
        return nullptr;
    }

    //Start the camera in free mode
    if (this->deviceLocked == DEVICE_FREE)
        this->startSync();

    //If camera is not ready we cannot capture images
    if (!this->camImageCapture->isReadyForCapture())
    {
        qDebug() << Q_FUNC_INFO << "Not ready for capture --- Camera state" << this->cam->state();
        return nullptr;
    }


    //Signal capture
    QEventLoop eventLoop;

    QObject::connect(this->camImageCapture, &QCameraImageCapture::imageAvailable,
                     &eventLoop, &QEventLoop::quit);
    //Error is overloaded so we need to use the old way of using connect
    QObject::connect(this->camImageCapture, SIGNAL(error(int,QCameraImageCapture::Error,QString)),
                     &eventLoop, SLOT(quit()));
    QObject::connect(this->camImageCapture, &QCameraImageCapture::readyForCaptureChanged,
                     &eventLoop, &QEventLoop::quit);
//    QObject::connect(this->camImageCapture, &QCameraImageCapture::imageSaved,
//                     &eventLoop, &QEventLoop::quit);


    /*
     * TODO: RETRIEVE IMAGE
     */
    //Bind the signal imageAvailable again but to a different function to save it
    QObject::connect(this->camImageCapture, &QCameraImageCapture::imageAvailable,
                     [=](int _id, const QVideoFrame &_image)
    {
        qDebug() << Q_FUNC_INFO << "ImageCaptured" << _id;
        //TODO: SAVE IMAGE HERE <<<<<<<
    });


    this->cam->searchAndLock();
    this->camImageCapture->capture();
    this->cam->unlock();

    eventLoop.exec(); // <<<<<<<<<<<<< Holds until signals in eventLoop


    if (this->camImageCapture->error() != QCameraImageCapture::NoError)
    {
        qDebug() << Q_FUNC_INFO << this->camImageCapture->errorString()  << this->cam->status();
        if (this->deviceLocked == DEVICE_FREE)
        {
            this->cam->stop();
        }
        return nullptr;
    }

    if (this->deviceLocked == DEVICE_FREE)
    {
        this->cam->stop();
    }

    //TODO: Return captured image
    return nullptr;
}


bool Camera::startSync()
{
    QEventLoop eventLoop;


    QObject::connect(this->camImageCapture, &QCameraImageCapture::readyForCaptureChanged,
                         &eventLoop, &QEventLoop::quit);
    //Error is overloaded so we need to use the old way of using connect
    QObject::connect(this->cam.get(), SIGNAL(error(QCamera::Error)),
                     &eventLoop, SLOT(quit()));


    qDebug() << Q_FUNC_INFO << "Ready " << this->camImageCapture->isReadyForCapture();
    this->cam->start();

    //We wait until the camera has changed stated or an error has been received
    eventLoop.exec();
    qDebug() << Q_FUNC_INFO << "Ready " << this->camImageCapture->isReadyForCapture();

    if (this->cam->error() != QCamera::NoError)
    {
        qDebug() << Q_FUNC_INFO << this->cam->errorString();
        return false;
    }

    return true;
}

//std::shared_ptr<CamImage> mat2png(cv::Mat* _img)
//{
//    CamImage output;

//    if (_img == nullptr)
//    {
//        qDebug() << "Cmat2png: Empty image";
//        return std::make_shared<CamImage> (output);
//    }

//    std::vector<int> param;
//    param.push_back(cv::IMWRITE_PNG_COMPRESSION);
//    param.push_back(9); //default(3) 0-9.

//    std::vector<unsigned char> out;
//    cv::imencode(".png", *_img, out, param);
//    output.appendData(out.data(),out.size());

//    return std::make_shared<CamImage> (output);

//}

