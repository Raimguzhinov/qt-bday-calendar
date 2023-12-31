#pragma once

#include <QImage>
#include <QUrl>

class NetworkingPrivate;

class Networking
{
public:
    static QImage httpGetImage(const QUrl &src);
    static void httpGetImageAsync(const QUrl &src, QObject *receiver, const char *slot);
    static bool insertPixmap(const QUrl &src, QObject *target, const QString &property = "pixmap");
    static QString httpGetString(const QUrl &src);
    static QString cookiePath();
    static void setCookiePath(const QString &path);

private:
    static NetworkingPrivate *networkingPrivate;
    static void init();
};
