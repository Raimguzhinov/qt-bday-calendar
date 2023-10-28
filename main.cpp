#include "calendar.hpp"
#include "connection.hpp"

#include <QApplication>
#include <QDate>
#include <QDesktopServices>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QSettings>
#include <QUrl>
#include <QVector>
#include <QWidget>

#include <iostream>
#include <string>

const char *secret = std::getenv("VK_CLIENT_SECRET");
const QUrl authUrl{"https://oauth.vk.com/authorize"};
const QUrl tokenUrl{"https://oauth.vk.com/access_token"};
const QString clientSecret{QString::fromUtf8(secret)};
const QString clientId{"51770901"};
constexpr quint32 scopeMask = 2;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QSettings settings("BirthdayCalendar", "GeneralSettings");
    bool isLocalhost = true;
    std::string docker_args = localhostConnection(isLocalhost, settings);
    if (docker_args == "Directory Error") {
        return 1;
    }

    QSqlDatabase db;
    errno_t errno_count = 0;
    do {
        db = createConnection(isLocalhost);
        if (errno_count > 4) {
            settings.clear();
            return 1;
        }
        if (isLocalhost || errno_count > 2) {
            std::chrono::seconds timespan(3);
            std::this_thread::sleep_for(timespan);
        }
        ++errno_count;
    } while (!checkConnection(&db));
    auto checkDBConnection = [&]() {
        if (!checkConnection(&db)) {
            db.close();
            if (!db.isValid()) {
                std::chrono::seconds timespan(10);
                std::this_thread::sleep_for(timespan);
                return;
            }
            db = createConnection(isLocalhost);
        }
    };
    QTimer timer;
    timer.setInterval(5000);
    QObject::connect(&timer, &QTimer::timeout, checkDBConnection);
    timer.start();

    Calendar w(nullptr, &db);
    QFile file(":/style.css");
    file.open(QFile::ReadOnly);
    w.setStyleSheet(file.readAll());
    w.setDockerPath(docker_args);
    w.show();

    QVariantList bdates, fios, friends_id, photos;
    qint64 my_id = 0;
    QString my_fio = "";
    auto oauth = new QOAuth2AuthorizationCodeFlow(&w);
    auto replyHandler = new QOAuthHttpServerReplyHandler(80, &w);
    w.setOauth(oauth);

    oauth->setReplyHandler(replyHandler);
    oauth->setAccessTokenUrl(tokenUrl);
    oauth->setAuthorizationUrl(authUrl);
    oauth->setClientIdentifier(clientId);
    oauth->setClientIdentifierSharedKey(clientSecret);
    oauth->setScope(QString::number(scopeMask));
    QObject::connect(oauth,
                     &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
                     &QDesktopServices::openUrl);
    QObject::connect(
        oauth,
        &QOAuth2AuthorizationCodeFlow::granted,
        [&w, &friends_id, &bdates, &fios, &photos, oauth]() {
            const QUrl getFriends{
                "https://api.vk.com/method/friends.get?fields=bdate,photo_200_orig"};
            auto network_reply = oauth->post(getFriends, {{"v", 5.131}});
            QObject::connect(
                network_reply,
                &QNetworkReply::finished,
                [&w, &friends_id, &bdates, &fios, &photos, network_reply]() {
                    network_reply->deleteLater();
                    QJsonDocument response = QJsonDocument::fromJson(network_reply->readAll());
                    qDebug() << "All my friends' birthdays:";
                    for (const auto &user_id : response["response"]["items"].toArray()) {
                        if (user_id.toObject()["bdate"].toString() != "") {
                            QString tmp_date_resolver = user_id.toObject()["bdate"].toString();
                            QStringList dateParts = tmp_date_resolver.split('.');
                            if (dateParts.length() == 2) {
                                QDate currentDate = QDate::currentDate();
                                int currentYear = currentDate.year();
                                tmp_date_resolver = QString("%1.%2.%3")
                                                        .arg(dateParts[0].toInt(), 2, 10, QChar('0'))
                                                        .arg(dateParts[1].toInt(), 2, 10, QChar('0'))
                                                        .arg(currentYear);
                            } else if (dateParts.length() == 3) {
                                int day = dateParts[0].toInt();
                                int month = dateParts[1].toInt();
                                int year = dateParts[2].toInt();
                                tmp_date_resolver = QString("%1.%2.%3")
                                                        .arg(day, 2, 10, QChar('0'))
                                                        .arg(month, 2, 10, QChar('0'))
                                                        .arg(year);
                            }
                            bdates.push_back(tmp_date_resolver);
                            photos.push_back(user_id.toObject()["photo_200_orig"].toString());
                            friends_id.push_back(user_id.toObject()["id"].toInteger());
                            fios.push_back(user_id.toObject()["first_name"].toString() + " "
                                           + user_id.toObject()["last_name"].toString());
                        }
                    }
                    w.setIDs(friends_id);
                    w.setBDays(bdates);
                    w.setPhotos(photos);
                    w.setFIOs(fios);
                });
        });
    QObject::connect(oauth, &QOAuth2AuthorizationCodeFlow::granted, [&w, &my_id, &my_fio, oauth]() {
        const QUrl getUser{"https://api.vk.com/method/users.get"};
        auto network_reply = oauth->post(getUser, {{"v", 5.131}});
        QObject::connect(network_reply,
                         &QNetworkReply::finished,
                         [&w, &my_id, &my_fio, network_reply] {
                             network_reply->deleteLater();
                             QJsonDocument response = QJsonDocument::fromJson(
                                 network_reply->readAll());
                             QJsonObject object = response["response"].toArray()[0].toObject();
                             my_id = object["id"].toInteger();
                             my_fio.append(object["first_name"].toString() + " "
                                           + object["last_name"].toString());
                             w.setMYID(my_id);
                             w.setMYFIO(my_fio);
                         });
    });
    return a.exec();
}
