#include "networking.hpp"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QImageReader>
#include <QNetworkCookie>
#include <QNetworkRequest>
#include <QPixmap>
#include <QVariant>

// internal header
#include "networking_p.hpp"

// class CookieJar - cookie storage
CookieJar::CookieJar()
    : QNetworkCookieJar()
{}

CookieJar::~CookieJar() {}

void CookieJar::loadCookies(const QString &cookiePath)
{
    QFile file(cookiePath + "/cookies.dat");
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray cookies = file.readAll();
        setAllCookies(QNetworkCookie::parseCookies(cookies));
    } else {
        qWarning() << "[CookieJar] Failed to load cookies: " + file.errorString();
    }
}

void CookieJar::saveCookies(const QString &cookiePath)
{
    QFile file(cookiePath + "/cookies.dat");
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QByteArray cookies;
        foreach (QNetworkCookie cookie, allCookies())
            cookies += cookie.toRawForm();
        file.write(cookies);
    } else {
        qWarning() << "[CookieJar] Failed to save cookies: " + file.errorString();
    }
}

// private class for real manipulations
NetworkingPrivate::NetworkingPrivate()
{
    nam = new QNetworkAccessManager();
    jar = new CookieJar();
    nam->setCookieJar(jar);
    loop = new QEventLoop();
    connect(nam, SIGNAL(finished(QNetworkReply *)), loop, SLOT(quit()));
    connect(nam, SIGNAL(finished(QNetworkReply *)), SLOT(onFinished(QNetworkReply *)));
    _cookiePath = qApp->applicationDirPath();
    jar->loadCookies(_cookiePath);
}

NetworkingPrivate::~NetworkingPrivate()
{
    jar->saveCookies(_cookiePath);
    nam->deleteLater();
    loop->deleteLater();
}

QImage NetworkingPrivate::httpGetImage(const QUrl &src) const
{
    QNetworkRequest request;
    request.setUrl(src);
    QNetworkReply *reply = nam->get(request);
    loop->exec();
    QVariant redirectedUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    QUrl redirectedTo = redirectedUrl.toUrl();
    if (redirectedTo.isValid()) {
        // guard from infinite redirect loop
        if (redirectedTo != reply->request().url()) {
            return httpGetImage(redirectedTo);
        } else {
            qWarning() << "[NetworkingPrivate] Infinite redirect loop at "
                              + redirectedTo.toString();
            return QImage();
        }
    } else {
        QImage img;
        QImageReader reader(reply);
        if (reply->error() == QNetworkReply::NoError)
            reader.read(&img);
        else
            qWarning() << QString("[NetworkingPrivate] Reply error: %1").arg(reply->error());
        reply->deleteLater();
        return img;
    }
}

void NetworkingPrivate::httpGetImageAsync(const QUrl &src, QObject *receiver, const char *slot)
{
    QNetworkRequest request;
    request.setUrl(src);
    QPair<QObject *, QPair<QUrl, const char *> > obj;
    obj.first = receiver;
    obj.second.first = src;
    obj.second.second = slot;
    QNetworkReply *reply = nam->get(request);
    requests.insert(reply, obj);
}

QString NetworkingPrivate::httpGetString(const QUrl &src) const
{
    QNetworkRequest request;
    request.setUrl(src);
    QNetworkReply *reply = nam->get(request);
    loop->exec();
    QVariant redirectedUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    QUrl redirectedTo = redirectedUrl.toUrl();
    if (redirectedTo.isValid()) {
        // guard from infinite redirect loop
        if (redirectedTo != reply->request().url()) {
            return httpGetString(redirectedTo);
        } else {
            qWarning() << "[NetworkingPrivate] Infinite redirect loop at "
                              + redirectedTo.toString();
            return "";
        }
    } else {
        QString answer;
        if (reply->error() == QNetworkReply::NoError)
            answer = QString::fromUtf8(reply->readAll());
        else
            qWarning() << QString("[NetworkingPrivate] Reply error: %1").arg(reply->error());
        reply->deleteLater();
        return answer;
    }
}

void NetworkingPrivate::setCookiePath(const QString &path)
{
    _cookiePath = path;
    jar->loadCookies(_cookiePath);
}

QString NetworkingPrivate::cookiePath() const
{
    return _cookiePath;
}

void NetworkingPrivate::onFinished(QNetworkReply *reply)
{
    if (requests.contains(reply)) {
        QPair<QObject *, QPair<QUrl, const char *> > obj = requests.value(reply);
        QVariant redirectedUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        QUrl redirectedTo = redirectedUrl.toUrl();
        if (redirectedTo.isValid()) {
            // guard from infinite redirect loop
            if (redirectedTo != reply->request().url()) {
                httpGetImageAsync(redirectedTo, obj.first, obj.second.second);
            } else {
                qWarning() << "[NetworkingPrivate] Infinite redirect loop at "
                                  + redirectedTo.toString();
            }
        } else {
            QImage img;
            QImageReader reader(reply);
            if (reply->error() == QNetworkReply::NoError)
                reader.read(&img);
            else
                qWarning() << QString("[NetworkingPrivate] Reply error: %1").arg(reply->error());
            if (obj.first && obj.second.second)
                QMetaObject::invokeMethod(obj.first,
                                          obj.second.second,
                                          Qt::DirectConnection,
                                          Q_ARG(QUrl, obj.second.first),
                                          Q_ARG(QImage, img));
        }
        requests.remove(reply);
        reply->deleteLater();
        jar->saveCookies(_cookiePath);
    }
}

// Networking class

NetworkingPrivate *Networking::networkingPrivate = 0;

QImage Networking::httpGetImage(const QUrl &src)
{
    init();
    return networkingPrivate->httpGetImage(src);
}

void Networking::httpGetImageAsync(const QUrl &src, QObject *receiver, const char *slot)
{
    init();
    networkingPrivate->httpGetImageAsync(src, receiver, slot);
}

bool Networking::insertPixmap(const QUrl &src, QObject *target, const QString &property)
{
    init();
    QImage img = httpGetImage(src);
    if (!img.isNull()) {
        return target->setProperty(property.toLatin1(), QVariant(QPixmap::fromImage(img)));
    }
    return false;
}

QString Networking::httpGetString(const QUrl &src)
{
    init();
    return networkingPrivate->httpGetString(src);
}

QString Networking::cookiePath()
{
    init();
    return networkingPrivate->cookiePath();
}

void Networking::setCookiePath(const QString &path)
{
    init();
    networkingPrivate->setCookiePath(path);
}

void Networking::init()
{
    if (!networkingPrivate)
        networkingPrivate = new NetworkingPrivate;
}
