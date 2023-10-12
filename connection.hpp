#pragma once

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QTextStream>
#include <QtSql>

inline bool createConnection()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setDatabaseName("postgres");
    db.setHostName("127.0.0.1");
    db.setPort(5433);
    db.setUserName("postgres");
    db.setPassword("postgres");
    if (!db.open()) {
        QMessageBox::warning(nullptr, "Ошибка БД", db.lastError().text());
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
