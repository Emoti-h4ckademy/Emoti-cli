#ifndef NETWORK_H
#define NETWORK_H

#include "camera.h"
#include <QString>


class Network
{
public:
    Network(QString _serverUrl);

    bool sendImage(std::shared_ptr<PngImage> _image, QString _username, QString _time);

private:
    QString serverUrl;

};

#endif // NETWORK_H
