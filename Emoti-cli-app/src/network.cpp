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

bool Network::sendImage(std::shared_ptr<CamImage> _image, QString _username, QString _time)
{
    // create custom temporary event loop on stack
    QEventLoop eventLoop;

    // "quit()" the event-loop, when the network request "finished()"
    QNetworkAccessManager mgr;
    QMetaObject::Connection connection = QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

    // the HTTP request
    QJsonObject json;
    json.insert("username", _username);
    json.insert("time",_time);

    QByteArray bimg (reinterpret_cast<const char*> (_image->getData().get()), _image->getSize());
    QString baseimg = bimg.toBase64();
    json.insert("image", baseimg);

    QJsonDocument jsonDoc;
    jsonDoc.setObject(json);

    qDebug() << "Network::sendImage POST DATA TO " << this->serverUrl << "  " << jsonDoc.toJson().size();

    QNetworkRequest req(QUrl(this->serverUrl));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    std::shared_ptr<QNetworkReply> reply (mgr.post(req, jsonDoc.toJson()));


    eventLoop.exec(); // blocks stack until "finished()" has been called
    eventLoop.disconnect(connection);

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << Q_FUNC_INFO << "Network::sendImage : Failure" <<reply->errorString() << "  " << jsonDoc.toJson().size();
        return false;
    }

    qDebug() << Q_FUNC_INFO << "Network::sendImage : Success" <<reply->readAll() << "  " << jsonDoc.toJson().size();
    return true;
}
