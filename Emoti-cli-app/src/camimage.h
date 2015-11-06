#ifndef CAMIMAGE_H
#define CAMIMAGE_H

#include <memory>

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
#endif // CAMIMAGE_H
