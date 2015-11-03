#include "network.h"

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>

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
    QUrlQuery postData;
    postData.addQueryItem("Username", _username);
    postData.addQueryItem("Time", _time);
   //postData.addQueryItem("PNG", _image->getData().get());

    QNetworkRequest req(QUrl(this->serverUrl));
    req.setHeader(QNetworkRequest::ContentTypeHeader,
                 "application/x-www-form-urlencoded");

    qDebug() << "Network::sendImage POST DATA TO " << this->serverUrl << "------- " << postData.toString(QUrl::FullyEncoded).toUtf8();

    QNetworkReply *reply = mgr.post(req, postData.toString(QUrl::FullyEncoded).toUtf8());


    eventLoop.exec(); // blocks stack until "finished()" has been called

    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "Network::sendImage : Success" <<reply->readAll();
        delete reply;
    }
    else {
        qDebug() << "Network::sendImage : Failure" <<reply->errorString();
        delete reply;
    }

    return true;
}
