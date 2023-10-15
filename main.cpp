#include "calendar.hpp"
#include "connection.hpp"

#include <QApplication>
#include <QSettings>

#include <QApplication>
#include <QDesktopServices>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
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
    QSettings settings("BirthdayCalendar", "MySettings");
    bool isLocalhost = false;
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
    Calendar w(nullptr, &db);
    w.setDockerPath(docker_args);
    QVector<QString> bdates, fios;
    QVector<qint64> friends_id;
    qint64 my_id = 0;
    QString my_fio = "";
    auto oauth = new QOAuth2AuthorizationCodeFlow(&w);
    auto replyHandler = new QOAuthHttpServerReplyHandler(80, &w);

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
        oauth, &QOAuth2AuthorizationCodeFlow::granted, [&w, &friends_id, &bdates, &fios, oauth]() {
            const QUrl getFriends{"https://api.vk.com/method/friends.get?fields=bdate"};
            auto network_reply = oauth->post(getFriends, {{"v", 5.131}});
            QObject::connect(
                network_reply,
                &QNetworkReply::finished,
                [&w, &friends_id, &bdates, &fios, network_reply]() {
                    network_reply->deleteLater();
                    QJsonDocument response = QJsonDocument::fromJson(network_reply->readAll());
                    qDebug() << "All my friends' birthdays:";
                    for (const auto &user_id : response["response"]["items"].toArray()) {
                        if (user_id.toObject()["bdate"].toString() != "") {
                            QString tmp_date_resolver = user_id.toObject()["bdate"].toString();
                            if (tmp_date_resolver.length() <= 5) {
                                tmp_date_resolver.append(".0000");
                                bdates.push_back(tmp_date_resolver);
                            } else {
                                bdates.push_back(user_id.toObject()["bdate"].toString());
                            }
                            friends_id.push_back(user_id.toObject()["id"].toInteger());
                            fios.push_back(user_id.toObject()["first_name"].toString() + " "
                                           + user_id.toObject()["last_name"].toString());
                        }
                    }
                    w.setFIOs(fios);
                    w.setBDays(bdates);
                    for (int i = 0; i < bdates.size(); ++i) {
                        qDebug() << friends_id[i] << "\t" << fios[i] << "\t" << bdates[i];
                    }
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
                             qDebug() << "My ID: " << my_id << "\t" << my_fio;
                         });
    });
    oauth->grant();
    w.show();
    return a.exec();
}
