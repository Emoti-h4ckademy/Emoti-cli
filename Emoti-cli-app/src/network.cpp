#include "network.h"

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonObject>
#include <QJsonDocument>

Network::Network(QString _serverUrl)
    :serverUrl(_serverUrl)
{
}

bool Network::sendImage(std::shared_ptr<PngImage> _image, QString _username, QString _time)
{
    // create custom temporary event loop on stack
    QEventLoop eventLoop;

    // "quit()" the event-loop, when the network request "finished()"
    QNetworkAccessManager mgr;
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

    // the HTTP request
    QJsonObject json;
    json.insert("Username", _username);
    json.insert("Time",_time);

    QByteArray bimg (reinterpret_cast<const char*> (_image->getData().get()), _image->getSize());
    QString baseimg = bimg.toBase64();
    json.insert("Image", baseimg);

    QJsonDocument jsonDoc;
    jsonDoc.setObject(json);



    qDebug() << "Network::sendImage POST DATA TO " << this->serverUrl << "------- " ;//<< jsonDoc.toJson();// << json;

    QNetworkRequest req(QUrl(this->serverUrl));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = mgr.post(req, jsonDoc.toJson());


    eventLoop.exec(); // blocks stack until "finished()" has been called

    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "Network::sendImage : Success" <<reply->readAll();
        delete reply;
    }
    else {
        qDebug() << "Network::sendImage : Failure" <<reply->errorString();
        delete reply;
        return false;
    }

    return true;
}
