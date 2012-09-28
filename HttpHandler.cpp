#include "HttpHandler.h"
#include <QUrl>
#include <QBuffer>
#include <QDebug>
#include <QImage>


HttpHandler::HttpHandler(const QString hostName, QObject *parent):
    QObject(parent), hostName(hostName)
{

    netManager = new QNetworkAccessManager(this);
    connect(netManager, SIGNAL(finished(QNetworkReply*) ), this, SLOT(postFinished(QNetworkReply*) ) );
}

void HttpHandler::sendImage(QImage image){
    //QNetworkRequest(QUrl(hostName+"/PhotoTheater/sendSnapshot.php"));


    QBuffer buffer( &ba );
    buffer.open( QBuffer::ReadWrite );
    image.save(&buffer, "PNG");
    QNetworkRequest request(QUrl(hostName+"/PhotoTheater/sendSnapshot.php"));

    request.setRawHeader("User-Agent", "PhotoTheater beta");
    //request.setAttribute();
    /*
    netManager->get(request);
    */
    netManager->post(request, ba);
    qDebug() << "Sending file";
}

void HttpHandler::postFinished(QNetworkReply* reply){
    QVariant possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

    /* We'll deduct if the redirection is valid in the redirectUrl function */
    qDebug() << "Redirect: " <<  possibleRedirectUrl.toUrl().toString();

    qDebug() << "File sent";
    QByteArray response = reply->readAll();
        printf("response: %s\n", response.data() );
        printf("reply error %d\n", reply->error() );
    reply->deleteLater();
    ba.clear();

}
