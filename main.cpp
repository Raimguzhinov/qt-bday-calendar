#include "calendar.hpp"
#include "connection.hpp"

#include <QApplication>
#include <QSettings>

#include <string>

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
    w.show();
    return a.exec();
}
