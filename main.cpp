#include "calendar.hpp"
#include "connection.hpp"

#include <QApplication>
#include <QProcess>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Calendar w;
    w.show();
    system("docker-compose up -d");
    do {
        std::chrono::milliseconds timespan(750);
        std::this_thread::sleep_for(timespan);
    } while (!createConnection());
    return a.exec();
}
