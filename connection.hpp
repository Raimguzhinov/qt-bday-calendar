#pragma once

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QString>
#include <QTextStream>
#include <QtSql>
#include <cstdlib>

QSqlDatabase createConnection(bool isLocalhost = false)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setDatabaseName("postgres");
    const char *hostname = std::getenv("DB_HOSTNAME");
    if (!isLocalhost && hostname) {
        db.setHostName(QString::fromUtf8(hostname));
    } else {
        db.setHostName("127.0.0.1");
    }
    db.setPort(5433);
    db.setUserName("postgres");
    db.setPassword("postgres");
    return db;
}

inline bool checkConnection(QSqlDatabase *db)
{
    if (!(*db).open()) {
        QMessageBox::warning(nullptr, "Ошибка БД", (*db).lastError().text());
        return false;
    } else {
        QMessageBox::information(nullptr, "Успешно", "Соединение с БД установлено!");
        return true;
    }
}

inline bool createDocker(QString filePath)
{
    QString fileName = filePath + "/docker-compose.yaml";
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << "version: '3.5'\n\n";
        stream << "name: mycalendar\n";
        stream << "services:\n";
        stream << "  db:\n";
        stream << "    restart: always\n";
        stream << "    container_name: postgres_container\n";
        stream << "    image: postgres:15.2\n";
        stream << "    environment:\n";
        stream << "      POSTGRES_USER: postgres\n";
        stream << "      POSTGRES_PASSWORD: postgres\n";
        stream << "      PGDATA: /data/postgres\n";
        stream << "      POSTGRES_DB: postgres\n";
        stream << "    volumes:\n";
        stream << "      - ./data/:/data/postgres\n";
        stream << "    ports:\n";
        stream << "      - \"5433:5432\"\n";
        file.close();
        std::string docker_args_up = "docker-compose -f " + fileName.toStdString() + " up -d";
        system(docker_args_up.c_str());
        return true;
    } else {
        return false;
    }
}

std::string localhostConnection(bool isLocalhost, QSettings &settings)
{
    std::string docker_args_down = "";
    if (isLocalhost) {
        QString defaultFilePath = settings.value("filePath").toString();
        QString filePath;
        if (defaultFilePath.isEmpty()) {
            filePath = QFileDialog::getExistingDirectory(nullptr,
                                                         "Выберите путь для сохранения файлов",
                                                         QString());
            if (filePath.isEmpty()) {
                QMessageBox::critical(nullptr, "Ошибка", "Не выбран путь для сохранения файла");
                return "Directory Error";
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
        docker_args_down = "docker-compose -f " + fileName.toStdString() + " down";

        system(docker_args_up.c_str());
    }
    return docker_args_down;
}
