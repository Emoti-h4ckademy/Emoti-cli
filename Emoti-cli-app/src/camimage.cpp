#include "camimage.h"

#include <QDebug>

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
        if (!np) goto allocatefailure;

        memcpy(np, _data, _size);

        this->data.reset(np, free);
        this->size = _size;
    } else {
        //Creates enough space for current + new data
        unsigned char *np = static_cast<unsigned char*> (malloc (nsize * sizeof(unsigned char)));
        if (!np) goto allocatefailure;

        memcpy(np, this->data.get(), this->size);
        memcpy(&np[this->size], _data, _size);

        this->data.reset(np, free);
        this->size = nsize;
    }

    return 0;

  allocatefailure:
    qDebug() << Q_FUNC_INFO << "Could not allocate memory";
    return 1;
}

