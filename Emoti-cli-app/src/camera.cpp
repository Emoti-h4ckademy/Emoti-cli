#include "camera.h"
#include <QDebug>

PngImage::PngImage()
    :data(nullptr), size(0)
{
}

size_t PngImage::getSize()
{
    return this->size;
}

std::shared_ptr<unsigned char> PngImage::getData()
{
    return this->data;
}

int PngImage::appendData(unsigned char *_data, size_t _size)
{
    size_t nsize = this->size + _size;

    if (this->data == nullptr)
    {
        //Create space
        unsigned char *np = static_cast<unsigned char*> (malloc (_size * sizeof(unsigned char)));
        if (!np) goto failure;

        memcpy(np, _data, _size);

        this->data.reset(np,free);
        this->size = _size;
    } else {

        //Embiggen space
        unsigned char *np = static_cast<unsigned char*> (malloc (nsize * sizeof(unsigned char)));
        if (!np) goto failure;

        memcpy(np, this->data.get(), this->size);
        memcpy(&np[this->size],_data, _size);

        this->data.reset(np,free);
        this->size = nsize;

    }

    return 0;

  failure:
    qDebug() << "PngImage::appendData - Could not allocate memory";
    return 1;
}

Camera::Camera()
    :cam(new cv::VideoCapture())
{
}

Camera::~Camera()
{
    if (this->cam->isOpened()) this->cam->release();
}

bool Camera::initCamera(int _device)
{
    bool check = this->cam->open(_device);

    if (!check)
        qDebug() << "Camera::initCamera - Device " << _device << " could not be open";

    return check;
}

std::shared_ptr<PngImage> Camera::getImage()
{
    if (!this->cam->isOpened())
    {
        qDebug() << "Camera::getImage - Device is not open";
        return nullptr;
    }

    cv::Mat frame;

    (*this->cam.get()) >> frame;

    return mat2png(&frame);

}

std::shared_ptr<PngImage> mat2png(cv::Mat* _img)
{
    PngImage output;

    if (_img == nullptr)
    {
        qDebug() << "Cmat2png: Empty image";
        return std::make_shared<PngImage> (output);
    }

    std::vector<int> param;
    param.push_back(cv::IMWRITE_PNG_COMPRESSION);
    param.push_back(9); //default(3) 0-9.

    std::vector<unsigned char> out;
    cv::imencode(".png", *_img, out, param);
    output.appendData(out.data(),out.size());

    return std::make_shared<PngImage> (output);

}

