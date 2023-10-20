#include "calendar.hpp"
#include "./ui_calendar.h"

Calendar::Calendar(QWidget *parent, QSqlDatabase *db)
    : QMainWindow(parent)
    , db_(*db)
    , ui(new Ui::Calendar)
{
    ui->setupUi(this);
    settings_ = new QSettings("BirthdayCalendar", "CalendarSettings", this);
    my_id_ = settings_->value("VK/my_id").toLongLong();
    my_fio_ = settings_->value("VK/my_fio").toString();
    ids_ = settings_->value("VK/ids").toList();
    fios_ = settings_->value("VK/fios").toList();
    bdates_ = settings_->value("VK/bdates").toList();

    query_ = new QSqlQuery(db_);
    query_->exec("SET datestyle TO ISO, DMY;");
    model_ = new QSqlRelationalTableModel(this, db_);
    model_->setEditStrategy(QSqlRelationalTableModel::OnManualSubmit);
    model_->setTable("birthdays");
    model_->setRelation(model_->fieldIndex("friend_vk_id"),
                        QSqlRelation("user_celebratings", "self_friends_id", "self_user_id"));
    if (my_id_ != 0) {
        setTotalInfo();
    }
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
    delete query_;
    delete model_;
    delete ui;
}

void Calendar::setIDs(QVariantList &ids)
{
    ids_ = ids;
    settings_->setValue("VK/ids", ids_);
    ids.clear();
}

void Calendar::setBDays(QVariantList &bdates)
{
    bdates_ = bdates;
    settings_->setValue("VK/bdates", bdates_);
    bdates.clear();
}

void Calendar::setFIOs(QVariantList &fios)
{
    fios_ = fios;
    settings_->setValue("VK/fios", fios_);
    setFriendsInfo();
    fios.clear();
}

void Calendar::setMYID(qint64 &my_id)
{
    my_id_ = my_id;
    settings_->setValue("VK/my_id", my_id_);
    my_id = 0;
}

void Calendar::setMYFIO(QString &my_fio)
{
    my_fio_ = my_fio;
    settings_->setValue("VK/my_fio", my_fio_);
    my_fio.clear();
}

void Calendar::setFriendsInfo()
{
    setMYInfo();
    if (!ids_.isEmpty() && !fios_.isEmpty() && !bdates_.isEmpty()) {
        qDebug() << ids_ << "\t" << fios_ << "\t" << bdates_;
        query_->prepare("INSERT INTO birthdays VALUES (?, ?, ?)");
        query_->addBindValue(ids_);
        query_->addBindValue(fios_);
        query_->addBindValue(bdates_);
        if (!query_->execBatch())
            qDebug() << query_->lastError();
        query_->prepare("INSERT INTO user_celebratings (self_user_id, self_friends_id) "
                        "VALUES (:self_user_id, :self_friends_id);");
        for (int i = 0; i < ids_.size(); ++i) {
            query_->bindValue(":self_user_id", my_id_);
            query_->bindValue(":self_friends_id", ids_[i]);
            query_->exec();
        }
    } else {
        ui->statusbar->showMessage("Предупреждение. Не удалось считать информацию о друзьях!");
    }
    setTotalInfo();
}

void Calendar::setMYInfo()
{
    if (my_fio_ != "" && my_id_ != 0) {
        qDebug() << "My ID: " << my_id_ << "\t" << my_fio_;
        query_->prepare("INSERT INTO users (user_vk_id, user_name) "
                        "VALUES (:user_vk_id, :user_name);");
        query_->bindValue(":user_vk_id", my_id_);
        query_->bindValue(":user_name", my_fio_);
        query_->exec();
    } else {
        ui->statusbar->showMessage("Предупреждение. Вы не авторизовались!");
    }
}

void Calendar::setTotalInfo()
{
    ui->label->clear();
    ui->tableView->clearSpans();
    model_->setFilter("self_user_id=" + QString::number(my_id_));
    model_->select();
    model_->setHeaderData(0, Qt::Horizontal, tr("мой id"));
    model_->setHeaderData(1, Qt::Horizontal, tr("мои друзья"));
    model_->setHeaderData(2, Qt::Horizontal, tr("дата рождения"));
    model_->setHeaderData(3, Qt::Horizontal, tr("описание"));
    ui->tableView->setModel(model_);
    ui->tableView->resizeColumnsToContents();
    ui->tableView->hideColumn(0);
    ui->label->setText(my_fio_);
    if (ui->label->text() != "account") {
        ui->sign_inButton->setDefault(false);
    }
}

void Calendar::setOauth(QOAuth2AuthorizationCodeFlow *oauth)
{
    oauth_ = oauth;
}

void Calendar::on_tableView_clicked(const QModelIndex &index)
{
    current_row_index_ = index.row();
    QVariant birthdayData = index.sibling(index.row(), 2).data();
    if (birthdayData.isValid() && birthdayData.type() == QVariant::Date) {
        QDate birthdayDate = birthdayData.toDate();
        getDate(birthdayDate);
        ui->calendarWidget->setSelectedDate(birthdayDate);
    }
}

void Calendar::on_sign_inButton_clicked()
{
    my_id_ = 0;
    my_fio_.clear();
    ids_.clear();
    fios_.clear();
    bdates_.clear();
    oauth_->grant();
}

void Calendar::on_calendarWidget_clicked(const QDate &date)
{
    getDate(date);
}

void Calendar::getDate(const QDate &date)
{
    ui->bithdayInfo->setPlainText("");
    query_ = new QSqlQuery(db_);
    if (!query_->exec("SELECT bdays.* FROM birthdays bdays LEFT JOIN user_celebratings uc ON "
                      "uc.self_friends_id = bdays.friend_vk_id WHERE uc.self_user_id="
                      + QString::number(my_id_)
                      + " AND DATE_PART('month', bday_date) = DATE_PART('month', '"
                      + date.toString("yyyy-MM-dd")
                      + "'::date) AND DATE_PART('day', bday_date) = DATE_PART('day', '"
                      + date.toString("yyyy-MM-dd") + "'::date);")) {
        qDebug() << query_->lastError().databaseText();
        qDebug() << query_->lastError().driverText();
    }
    while (query_->next()) {
        QString birthdayString = "День рождения у " + query_->record().value(1).toString() + " "
                                 + QLocale(QLocale::Russian).toString(date, "d MMMM");
        ui->bithdayInfo->append(birthdayString);
    }
}
