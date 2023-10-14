#include "calendar.hpp"
#include "./ui_calendar.h"

Calendar::Calendar(QWidget *parent, QSqlDatabase *db)
    : QMainWindow(parent)
    , db_(*db)
    , ui(new Ui::Calendar)
{
    ui->setupUi(this);
    query_ = new QSqlQuery(db_);
    query_->exec("SELECT name FROM sqlite_master WHERE type='table' AND name='birthdays'");
    if (!query_->next()) {
        query_->exec("CREATE TABLE birthdays (fio TEXT, date DATE NOT NULL)");
    }
    if (query_->lastError().isValid()) {
        qDebug() << "Table creation error:" << query_->lastError().text();
    }
    model_ = new QSqlTableModel(this, db_);
    model_->setQuery("SELECT * FROM birthdays");
    model_->setTable("birthdays");
    model_->select();
    ui->textEdit->setPlainText(model_->tableName());
    ui->tableView->setModel(model_);
}

void Calendar::setDockerPath(std::string docker_args_down)
{
    docker_path_ = docker_args_down;
}

Calendar::~Calendar()
{
    if (!docker_path_.empty()) {
        system(docker_path_.c_str());
    }
    delete ui;
}

void Calendar::on_pushButton_clicked()
{
    query_ = new QSqlQuery(db_);
    if (!query_->exec("SELECT * FROM Birthdays")) {
        qDebug() << query_->lastError().databaseText();
        qDebug() << query_->lastError().driverText();
        return;
    }
    while (query_->next()) {
        qDebug() << query_->record();
    }
}

void Calendar::on_pushButton_2_clicked()
{
    model_->insertRow(model_->rowCount());
}

void Calendar::on_pushButton_3_clicked()
{
    model_->removeRow(current_row_index_);
    model_->select();
}

void Calendar::on_tableView_clicked(const QModelIndex &index)
{
    current_row_index_ = index.row();
}
