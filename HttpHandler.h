#ifndef HTTPHANDLER_H
#define HTTPHANDLER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QImage>

class HttpHandler : public QObject
{
    Q_OBJECT
public:
    explicit HttpHandler(const QString hostName, QObject *parent = 0);
    void sendImage(QImage image);

protected:
    QNetworkAccessManager * netManager;
    QString hostName;
    QByteArray ba;

signals:

public slots:
    void postFinished(QNetworkReply*);

};

#endif // HTTPHANDLER_H
