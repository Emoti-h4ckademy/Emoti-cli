#ifndef CAMERA_H
#define CAMERA_H

#include <memory>
#include <QCamera>
#include <QCameraImageCapture>
#include <QCameraInfo>

#include "camimage.h"


/**
 * @brief Class to handle the camera
 */
class Camera
{
public:
    Camera();
    ~Camera();

    /**
     * @brief Used to determine wether we use the camera continuously (DEVICE LOCK) or,
     * on other hand, open the camera, take a picture and close id (DEVICE FREE)
     */
    enum lockStatus {DEVICE_LOCK, DEVICE_FREE};

    /**
     * @brief setup Initialize the camera to the required values
     * @param _device Computers device to use
     * @param _deviceLocked Check enum lockStatus (use camera always or not)
     * @return 0 if OK, !0 if errors
     * Logs with qDebug
     * In case of errors it keeps the previous state
     */
    int setup (QCameraInfo& _device, lockStatus _deviceLocked = DEVICE_FREE);

    /**
     * @brief changeDevice Changes the used device to capture images
     * @param _device New device to be used
     * @return 0 if OK, !0 if error.
     * Logs with qDebug
     */
    int setDevice (QCameraInfo& _device);

    int setDeviceMode (lockStatus _deviceLocked);

    /**
     * @brief captureImageSync Captures an image synchronously
     * (waits until the image is ready or until it finds an error to return)
     * @return Shared_ptr to the image captured. Nullptr if it wasn't possible to capture it.
     * Logs with qDebug
     */
    std::shared_ptr<CamImage> captureImageSync();

    int start(bool _firstTime);
    int stop(bool _forceStop);


private:
    std::shared_ptr<QCamera> cam;
    QCameraImageCapture *camImageCapture;
    lockStatus deviceLocked;

    /**
     * @brief startSync Starts the camera.
     * Wait until either an error is found or capturing is possible.
     * @return true -> camera ready; false -> error.
     * Logs with qDebug
     */
    bool startSync();
};

/**
 * @brief mat2png Transform a cvMat into a PngImage
 * @param _img
 * @return
 */
//std::shared_ptr<CamImage> mat2png(cv::Mat* _img);


#endif // CAMERA_H

