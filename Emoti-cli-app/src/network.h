#ifndef NETWORK_H
#define NETWORK_H

#include "camimage.h"
#include <QString>


class Network
{
public:
    /**
     * @brief Network Class to handle network connections
     * @param _serverUrl - Url (protocol included) to the server
     */
    explicit Network(QString _serverUrl);

    /**
     * @brief sendImage Sends a CamImage
     * @param _image Image to be passed as data
     * @param _username Username to be passed as data
     * @param _time Image timestamp to be passed as data
     * @return
     */
    bool sendImage(std::shared_ptr<CamImage> _image, QString _username, QString _time);

private:
    QString serverUrl;

};

#endif // NETWORK_H
