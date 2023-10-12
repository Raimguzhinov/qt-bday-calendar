#pragma once

#include <QMessageBox>
#include <QSqlDatabase>
#include <QtSql>

inline bool createConnection()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setDatabaseName("birthdays");
    db.setHostName("127.0.0.1");
    db.setPort(5433);
    db.setUserName("guest");
    db.setPassword("hello123");
    if (!db.open()) {
        QMessageBox::warning(nullptr, "Ошибка БД", db.lastError().text());
        return false;
    } else {
        QMessageBox::information(nullptr, "Успешно", "Соединение с БД установлено!");
        return true;
    }
}
