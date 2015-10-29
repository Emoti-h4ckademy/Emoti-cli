#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/opencv.hpp>
#include <memory>

class Camera
{
public:
    Camera();

    bool initCamera (int _device);
    std::shared_ptr<cv::Mat> getImage();

private:
    std::shared_ptr<cv::VideoCapture> cam;
};


#endif // CAMERA_H

