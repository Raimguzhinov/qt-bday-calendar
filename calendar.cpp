#include "calendar.hpp"
#include "./ui_calendar.h"

Calendar::Calendar(QWidget *parent, QSqlDatabase *db)
    : QMainWindow(parent)
    , db_(*db)
    , ui(new Ui::Calendar)
{
    ui->setupUi(this);
    //query_ = new QSqlQuery(db_);
    model_ = new QSqlRelationalTableModel(this, db_);
    model_->setEditStrategy(QSqlRelationalTableModel::OnManualSubmit);
    model_->setTable("birthdays");
    model_->setRelation(model_->fieldIndex("friend_vk_id"),
                        QSqlRelation("user_celebratings", "self_friends_id", "self_user_id"));
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
    if (!query_->exec(
            "select bdays.* from birthdays bdays left join "
            "user_celebratings uc on uc.self_friends_id=bdays.friend_vk_id where uc.self_user_id="
            + QString::number(my_id_))) {
        qDebug() << query_->lastError().databaseText();
        qDebug() << query_->lastError().driverText();
        return;
    }
    while (query_->next()) {
        qDebug() << query_->record();
    }
}

void Calendar::setBDays(QVector<QString> bdates)
{
    bdates_ = bdates;
}

void Calendar::setFIOs(QVector<QString> fios)
{
    fios_ = fios;
}

void Calendar::setIDs(QVector<qint64> ids)
{
    ids_ = ids;
}

void Calendar::setMYID(qint64 my_id)
{
    my_id_ = my_id;
    model_->setFilter("self_user_id=" + QString::number(my_id_));
    model_->select();
    model_->setHeaderData(0, Qt::Horizontal, tr("мой id"));
    model_->setHeaderData(1, Qt::Horizontal, tr("мои друзья"));
    model_->setHeaderData(2, Qt::Horizontal, tr("дата рождения"));
    model_->setHeaderData(3, Qt::Horizontal, tr("описание"));
    ui->tableView->setModel(model_);
    ui->tableView->hideColumn(0);
}
void Calendar::setMYFIO(QString my_fio)
{
    my_fio_ = my_fio;
    ui->textEdit->setPlainText(my_fio_);
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

void Calendar::on_pushButton_4_clicked() {}

void Calendar::on_pushButton_6_clicked()
{
    model_->submit();
}
