#include "camimage.h"

#include <QDebug>
#include <QBuffer>
#include <QPixmap>

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

std::shared_ptr<CamImage> qImage2CamImage (QImage &_src, const char *_format)
{
    QByteArray byteArray;
    QBuffer bufferAux(&byteArray);
    _src.save(&bufferAux, _format);

    CamImage camAux;
    camAux.appendData(reinterpret_cast<unsigned char*> (byteArray.data()), byteArray.size());

    return std::make_shared<CamImage> (camAux);
}


std::shared_ptr<CamImage> qVideoFrameJPEG2CamImage (QVideoFrame &_src, const char *_format)
{
    QImage qImageAux;
    QPixmap pix;
    pix.loadFromData(_src.bits(), _src.mappedBytes(), "JPG");
    qImageAux = pix.toImage();

    return qImage2CamImage(qImageAux, _format);
}

std::shared_ptr<CamImage> qVideoFrame2CamImage (QVideoFrame &_src, const char *_format)
{
    std::shared_ptr<CamImage> output;

    //MAP frame into memory
    if (! _src.map(QAbstractVideoBuffer::ReadOnly))
    {
        qDebug() << Q_FUNC_INFO << "Could not map the VideoFrame";
        return nullptr;
    }

    if (_src.pixelFormat() == QVideoFrame::Format_Jpeg)
    {
        output =  qVideoFrameJPEG2CamImage(_src, _format);
    }
    else
    {
        QImage::Format imgFormat = QVideoFrame::imageFormatFromPixelFormat(_src.pixelFormat());
        QImage qImageAux = QImage(_src.bits(), _src.width(), _src.height(), imgFormat);
        output = qImage2CamImage (qImageAux, _format);
    }

    //UNMAP frame into memory
    _src.unmap();

    return output;
}
