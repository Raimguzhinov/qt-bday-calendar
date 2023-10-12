#include "calendar.hpp"
#include "connection.hpp"

#include <QApplication>
#include <QSettings>

#include <string>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Calendar w;
    w.show();
    QSettings settings("BirthdayCalendar", "MySettings");
    QString defaultFilePath = settings.value("filePath").toString();
    QString filePath;
    if (defaultFilePath.isEmpty()) {
        filePath = QFileDialog::getExistingDirectory(nullptr,
                                                     "Выберите путь для сохранения файлов",
                                                     QString());
        if (filePath.isEmpty()) {
            QMessageBox::critical(nullptr, "Ошибка", "Не выбран путь для сохранения файла");
            return 1;
        }
        settings.setValue("filePath", filePath);
        if (!createDocker(filePath)) {
            QMessageBox::critical(nullptr, "Ошибка", "Не удалось открыть файл для записи");
        } else {
            QMessageBox::information(nullptr, "Успех", "Файл успешно записан");
        }
    } else {
        filePath = defaultFilePath;
    }

    QString fileName = filePath + "/docker-compose.yaml";
    std::string docker_args_up = "docker-compose -f " + fileName.toStdString() + " up -d";
    std::string docker_args_down = "docker-compose -f " + fileName.toStdString() + " down";
    w.setDockerPath(docker_args_down);
    system(docker_args_up.c_str());

    errno_t errno_count = 0;
    do {
        if (errno_count > 2) {
            settings.clear();
            return 1;
        }
        std::chrono::seconds timespan(3);
        std::this_thread::sleep_for(timespan);
        ++errno_count;
    } while (!createConnection());
    return a.exec();
}
