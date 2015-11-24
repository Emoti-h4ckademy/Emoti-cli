#ifndef CAMERA_H
#define CAMERA_H

#include <memory>
#include <QCamera>
#include <QCameraImageCapture>
#include <QCameraInfo>
#include <QMutex>

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
     * @brief Used to determine wether we use the camera continuously (DEVICE LOCKED) or,
     * on other hand, open the camera, take a picture and close id (DEVICE FREE)
     */
    enum lockStatus {DEVICE_LOCKED, DEVICE_FREE};

    /**
     * @brief Due to some cameras not being able to store the generated image from QCameraImageCapture
     * directly to memory, it's needed a temporal file to store and then read it. If memory is selected
     * but the device is imcompatible, it will still use a file.
     */
    enum imageDestination {DESTINATION_MEMORY, DESTINATION_FILE};

    /**
     * @brief setup Initialize the camera to the required values
     * @param _device Computers device to use
     * @param _deviceLocked Check enum lockStatus (use camera always or not)
     * @param _tempDestination Check emum imageDestination (use memory or temp file while handling the images)
     * @return 0 if OK, !0 if errors
     * Logs with qDebug
     * In case of errors it keeps the previous state
     */
    int setup (QCameraInfo& _device, lockStatus _deviceLocked = DEVICE_FREE, imageDestination _tempDestination = DESTINATION_MEMORY);

    /**
     * @brief changeDevice Changes the used device to capture images
     * @param _device New device to be used
     * @param _tempDestination Check emum imageDestination (use memory or temp file while handling the images)
     * @return 0 if OK, !0 if error.
     * Logs with qDebug
     */
    int setDevice (QCameraInfo& _device, imageDestination _tempDestination);

    int setDeviceMode (lockStatus _deviceLocked);

    /**
     * @brief captureImageSync Captures an image synchronously
     * (waits until the image is ready or until it finds an error to return)
     * A mutex is used to be sure only one image is used at a time
     * @param _format - Format of the image to be returned
     * @return Shared_ptr to the image captured. Nullptr if it wasn't possible to capture it.
     * Logs with qDebug
     */
    std::shared_ptr<CamImage> captureImageSync(const char* _format = "png");


private:
    std::shared_ptr<QCamera> cam;
    QCameraImageCapture *camImageCapture;
    lockStatus deviceLocked;
    QBasicMutex camMutex;
    imageDestination camDestination;
    const int TIMEOUTs = 6;

    //Template to use in the temp file
    const QString fTemplate;

    //Helpers to differenciate between capturing with the camera with a temp file or using just memory.
    //Checks and resource locks are not done.
    std::shared_ptr<CamImage> captureImageSync_memory(const char *_format);
    std::shared_ptr<CamImage> captureImageSync_file(const char *_format);

    /**
     * @brief start Starts the camera (if needed) to capture images
     * @param _firstTime Wether this camera has just been initialized
     * @return 0 if OK, !0 if error.
     * Logs with qDebug
     */
    int start(bool _firstTime);

    /**
     * @brief stop Stops the camera (if needed) after capturing an image
     * @param _forceStop Stops the camera no mather the lockStatus it has
     * @return 0 if OK, !0 if error.
     * Logs with qDebug
     */
    int stop(bool _forceStop);

    /**
     * @brief startSync Starts the camera.
     * Wait until either an error is found or capturing is possible.
     * @return true -> camera ready; false -> error.
     * Logs with qDebug
     */
    bool startSync();
};


#endif // CAMERA_H

