#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/opencv.hpp>
#include <memory>


struct mem_encode
{
  char* buffer = NULL;
  size_t size = 0;
  size_t allocd = 0;
};

std::shared_ptr<struct mem_encode*> mat2png(cv::Mat &_img);

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

