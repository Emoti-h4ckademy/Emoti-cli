#ifndef CAMIMAGE_H
#define CAMIMAGE_H

#include <memory>
#include <QVideoFrame>

/**
 * @brief Class to manage an image from the camera
 */
class CamImage
{
public:
    CamImage();
    size_t getSize();
    std::shared_ptr<unsigned char> getData();

    /**
     * @brief appendData Adds data to the back of the image
     * @param _data
     * @param _size
     * @return 0 if OK, 1 if memory could not be allocated
     * Logs with qDebug
     */
    int appendData (unsigned char* _data, size_t _size);

private:
    std::shared_ptr<unsigned char> data;
    size_t size;

};

/**
 * @brief qImage2CamImage Converts a QImage into a CamImage
 * @param _src QImage source
 * @param _format Format to be given to the CamImage, by default PNG
 * @return shrptr to the image or nullptr
 * Logs with qDebug
 */
std::shared_ptr<CamImage> qImage2CamImage (QImage &_src, const char* _format = "PNG");


/**
 * @brief qVideoFrame2CamImage Converts a QVideoFrame into a CamImage
 * @param _src QVideoFrame source
 * @param _format Format to be given to the CamImage, by default PNG
 * @return shrptr to the image or nullptr
 * Logs with qDebug
 */
std::shared_ptr<CamImage> qVideoFrame2CamImage (QVideoFrame &_src, const char* _format = "PNG");

#endif // CAMIMAGE_H
