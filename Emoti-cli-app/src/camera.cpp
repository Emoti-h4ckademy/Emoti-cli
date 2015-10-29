#include "camera.h"
#include <QApplication>

Camera::Camera()
    :cam(new cv::VideoCapture())
{
}

bool Camera::initCamera(int _device)
{
    return this->cam->open(_device);
}

std::shared_ptr<cv::Mat> Camera::getImage()
{
    if (!this->cam->isOpened())
    {
        QObject::tr("Camera is not accesible");
        return nullptr;
    }

    std::shared_ptr<cv::Mat> frame (new cv::Mat());
    cam->operator >> (*frame);

    return frame;

}

