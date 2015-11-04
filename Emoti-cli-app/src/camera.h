#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/opencv.hpp>
#include <memory>

/**
 * @brief Class to manage a PNG Image
 */
class PngImage
{
public:
    PngImage();
    size_t getSize();
    std::shared_ptr<unsigned char> getData();

    /**
     * @brief appendData Adds data to the back of the image
     * @param _data
     * @param _size
     * @return 1 if memory could not be allocated
     */
    int appendData (unsigned char* _data, size_t _size);

private:
    std::shared_ptr<unsigned char> data;
    size_t size;

};

class Camera
{
public:
    Camera();
    ~Camera();

    bool initCamera (int _device);
    std::shared_ptr<PngImage>  getImage();

private:
    std::shared_ptr<cv::VideoCapture> cam;
};

/**
 * @brief mat2png Transform a cvMat into a PngImage
 * @param _img
 * @return
 */
std::shared_ptr<PngImage> mat2png(cv::Mat* _img);


#endif // CAMERA_H

