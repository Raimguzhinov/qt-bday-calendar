#include "calendar.hpp"
#include "./ui_calendar.h"
#include "helpinformation.hpp"
#include "imagemanager.hpp"
#include "networking.hpp"
#include <utility>

#define I2P(image) QPixmap::fromImage(image)

Calendar::Calendar(QWidget *parent, QSqlDatabase *db)
    : QMainWindow(parent)
    , db_(*db)
    , ui(new Ui::Calendar)
{
    ui->setupUi(this);
    ui->calendarWidget->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    ui->calendarWidget->setGridVisible(false);
    ui->searchInput->setClearButtonEnabled(true);
    ui->searchInput->addAction(QIcon(":/resources/images/search.ico"),
                               QLineEdit::ActionPosition::LeadingPosition);
    settings_ = new QSettings("BirthdayCalendar", "CalendarSettings", this);
    my_id_ = settings_->value("VK/my_id").toLongLong();
    my_fio_ = settings_->value("VK/my_fio").toString();
    ids_ = settings_->value("VK/ids").toList();
    fios_ = settings_->value("VK/fios").toList();
    bdates_ = settings_->value("VK/bdates").toList();
    photos_ = settings_->value("VK/photos").toList();
    connect(ui->action_2, SIGNAL(triggered()), this, SLOT(log_out()));
    resetImage();
    query_ = new QSqlQuery(db_);
    query_->exec("SET datestyle = 'ISO, DMY';");
    model_ = new QSqlRelationalTableModel(this, db_);
    model_->setEditStrategy(QSqlRelationalTableModel::OnManualSubmit);
    model_->setTable("birthdays");
    model_->setRelation(model_->fieldIndex("friend_vk_id"),
                        QSqlRelation("user_celebratings", "self_friends_id", "self_user_id"));
    if (my_id_ != 0) {
        setTotalInfo();
    } else {
        QTimer timer;
        timer.setInterval(5000);
        connect(&timer, &QTimer::timeout, this, &Calendar::on_sign_inButton_clicked);
        timer.start();
    }
    this->activateWindow();
    this->setFocus();
    connect(ui->action_4, SIGNAL(triggered()), this, SLOT(slotInfo()));
    connect(ui->action_5, SIGNAL(triggered()), this, SLOT(slotAbout()));
    keyF11 = new QShortcut(this);
    keyF11->setKey(Qt::Key_F11);
    keyCtrlL = new QShortcut(this);
    keyCtrlL->setKey(Qt::CTRL + Qt::Key_L);
    keyCtrlO = new QShortcut(this);
    keyCtrlO->setKey(Qt::CTRL + Qt::Key_O);
    keyCtrlI = new QShortcut(this);
    keyCtrlI->setKey(Qt::CTRL + Qt::Key_I);
    keyCtrlB = new QShortcut(this);
    keyCtrlB->setKey(Qt::CTRL + Qt::Key_B);
    connect(keyF11, SIGNAL(activated()), this, SLOT(slotShortcutF11()));
    connect(keyCtrlL, SIGNAL(activated()), this, SLOT(log_out()));
    connect(keyCtrlO, SIGNAL(activated()), this, SLOT(on_sign_inButton_clicked()));
    connect(keyCtrlI, SIGNAL(activated()), this, SLOT(slotInfo()));
    connect(keyCtrlB, SIGNAL(activated()), this, SLOT(slotAbout()));
}

void Calendar::setDockerPath(std::string docker_args_down) {
  docker_path_ = docker_args_down;
}

Calendar::~Calendar() {
  if (!docker_path_.empty()) {
    system(docker_path_.c_str());
  }
  delete query_;
  delete model_;
  delete ui;
}

void Calendar::setIDs(QVariantList &ids) {
  ids_ = ids;
  settings_->setValue("VK/ids", ids_);
  ids.clear();
}

void Calendar::setBDays(QVariantList &bdates) {
  bdates_ = bdates;
  settings_->setValue("VK/bdates", bdates_);
  bdates.clear();
}

void Calendar::setFIOs(QVariantList &fios) {
  fios_ = fios;
  settings_->setValue("VK/fios", fios_);
  setFriendsInfo();
  fios.clear();
}

void Calendar::setMYID(qint64 &my_id) {
  my_id_ = my_id;
  settings_->setValue("VK/my_id", my_id_);
  my_id = 0;
}

void Calendar::setMYFIO(QString &my_fio) {
  my_fio_ = my_fio;
  settings_->setValue("VK/my_fio", my_fio_);
  my_fio.clear();
}

void Calendar::setPhotos(QVariantList &photos) {
  photos_ = photos;
  settings_->setValue("VK/photos", photos_);
  photos.clear();
}

void Calendar::setFriendsInfo() {
  setMYInfo();
  if (!ids_.isEmpty() && !fios_.isEmpty() && !bdates_.isEmpty()) {
    my_ids_.fill(my_id_, ids_.size());
    qDebug() << ids_ << "\t" << fios_ << "\t" << bdates_;
    query_->prepare(
        "INSERT INTO birthdays VALUES (?, ?, to_date(?, 'DD-MM-YYYY'), ?) ON "
        "CONFLICT DO NOTHING");
    query_->addBindValue(ids_);
    query_->addBindValue(fios_);
    query_->addBindValue(bdates_);
    query_->addBindValue(photos_);
    if (!query_->execBatch())
      qDebug() << query_->lastError();
    query_->prepare("INSERT INTO user_celebratings VALUES (?, ?)");
    query_->addBindValue(my_ids_);
    query_->addBindValue(ids_);
    if (!query_->execBatch())
      qDebug() << query_->lastError();
  } else {
    ui->statusbar->showMessage(
        "Предупреждение. Не удалось считать информацию о друзьях!");
  }
  setTotalInfo();
}

void Calendar::setMYInfo() {
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

void Calendar::setTotalInfo() {
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
  ui->tableView->hideColumn(3);
  ui->tableView->hideColumn(4);
  ui->label->setText(my_fio_);
  if (ui->label->text() != "Добро пожаловать!") {
    ui->sign_inButton->setDefault(false);
  }
  this->activateWindow();
  this->setFocus();
}

void Calendar::setOauth(QOAuth2AuthorizationCodeFlow *oauth) { oauth_ = oauth; }

void Calendar::loadImage(const QString &urlString) {
  Networking::httpGetImageAsync(QUrl(urlString), this, "onImageRead");
}

void Calendar::setImage(const QImage &image) {
  currentImage_ = ImageManager::normallyResized(image, 150);
  updateImages();
}

void Calendar::resetImage() {
  setImage(QImage(":/resources/images/anonim.jpg"));
}

void Calendar::updateImages() {
  ui->label_2->setPixmap(
      I2P(ImageManager::roundSquared(currentImage_, 147, 12)));
}

void Calendar::on_tableView_clicked(const QModelIndex &index) {
  current_row_fio_ = index.sibling(index.row(), 1).data().toString();
  QVariant birthdayData = index.sibling(index.row(), 2).data();
  if (birthdayData.isValid() && birthdayData.type() == QVariant::Date) {
    current_birthday_date_ = birthdayData.toDate();
    getDate(current_birthday_date_, current_birthday_date_);
    ui->calendarWidget->setSelectedDate(current_birthday_date_);
  }
  query_->prepare(
      "SELECT bdays.* FROM birthdays bdays LEFT JOIN user_celebratings uc "
      "ON "
      "uc.self_friends_id = bdays.friend_vk_id WHERE bdays.friend_name="
      ":friend_name;");
  query_->bindValue(":friend_name", current_row_fio_);
  if (!query_->exec()) {
    qDebug() << query_->lastError().databaseText();
    qDebug() << query_->lastError().driverText();
  }
  query_->next();
  loadImage(query_->record().value(3).toString());
}

void Calendar::on_sign_inButton_clicked() {
  my_id_ = 0;
  my_fio_.clear();
  ids_.clear();
  fios_.clear();
  bdates_.clear();
  oauth_->grant();
  this->activateWindow();
  this->setFocus();
}

QDate firstDate;
void Calendar::on_calendarWidget_clicked(const QDate &date)
{
  resetImage();
  if (firstDate.isNull()) {
    clearDateTextFormats();
    firstDate = date;
    getDate(firstDate, firstDate);
  } else {
    showDateRange(firstDate, date);
    firstDate = QDate();
  }
}

void Calendar::showDateRange(const QDate &startDate, const QDate &endDate)
{
  QDate tmpStartDate = startDate;
  QDate tmpEndDate = endDate;
  if (tmpStartDate > tmpEndDate) {
    QDate tmp = tmpStartDate;
    tmpStartDate.setDate(tmpEndDate.year(), tmpEndDate.month(), tmpEndDate.day());
    tmpEndDate.setDate(tmp.year(), tmp.month(), tmp.day());
  }
  getDate(tmpStartDate, tmpEndDate);
  QTextCharFormat format;
  format.setBackground(Qt::darkCyan);
  for (QDate d = tmpStartDate; d <= tmpEndDate; d = d.addDays(1)) {
    ui->calendarWidget->setDateTextFormat(d, format);
  }
}

void Calendar::clearDateTextFormats()
{
  QTextCharFormat format;
  QDate firstDayOfMonth(ui->calendarWidget->selectedDate().year(),
                        ui->calendarWidget->selectedDate().month(),
                        1);
  QDate lastDayOfMonth = firstDayOfMonth.addMonths(2).addDays(-1);
  for (QDate date = firstDayOfMonth.addMonths(-1); date <= lastDayOfMonth; date = date.addDays(1)) {
    ui->calendarWidget->setDateTextFormat(date, format);
  }
}

void Calendar::getDate(const QDate &startDate, const QDate &endDate)
{
  ui->bithdayInfo->setPlainText("");
  query_->prepare("SELECT bdays.* FROM birthdays bdays LEFT JOIN user_celebratings uc "
                  "ON uc.self_friends_id = bdays.friend_vk_id WHERE "
                  "uc.self_user_id=:self_user_id AND "
                  "DATE_PART('month', bday_date) = DATE_PART('month', :startDate::date) AND "
                  "DATE_PART('day', bday_date) BETWEEN DATE_PART('day', :startDate::date) AND "
                  "DATE_PART('day', :endDate::date)");
  query_->bindValue(":self_user_id", QString::number(my_id_));
  query_->bindValue(":startDate", startDate.toString("yyyy-MM-dd"));
  query_->bindValue(":endDate", endDate.toString("yyyy-MM-dd"));

  if (!query_->exec()) {
    qDebug() << query_->lastError().databaseText();
    qDebug() << query_->lastError().driverText();
  }
  int cnt = 1;
  while (query_->next()) {
    QString name = query_->record().value(1).toString();
    QString birthdayString = name + " празднует "
                             + QLocale(QLocale::Russian)
                                   .toString(query_->record().value(2).toDate(), "d MMMM")
                             + "\nЗаметка: ";
    birthdayString += query_->record().value(4).toString().isEmpty()
                          ? "[-]"
                          : "[+] " + query_->record().value(4).toString();
    ui->bithdayInfo->append(birthdayString + "\n");
    if (cnt > 1) {
      continue;
    } else {
      loadImage(query_->record().value(3).toString());
    }
    ++cnt;
  }
}

void Calendar::onImageRead(const QUrl &imageUrl, const QImage &image) {
  Q_UNUSED(imageUrl)
  setImage(image);
}

void Calendar::on_submitButton_clicked() {
  query_->prepare("UPDATE birthdays SET description=:description WHERE "
                  "friend_name=:friend_name");
  query_->bindValue(":description", ui->lineEdit->text());
  query_->bindValue(":friend_name", current_row_fio_);
  if (!query_->exec()) {
    qDebug() << query_->lastError().databaseText();
    qDebug() << query_->lastError().driverText();
  }
  model_->submitAll();
  setTotalInfo();
  ui->lineEdit->clear();
  emit on_calendarWidget_clicked(current_birthday_date_);
}

void Calendar::on_lineEdit_returnPressed() {
  query_->prepare("UPDATE birthdays SET description=:description WHERE "
                  "friend_name=:friend_name");
  query_->bindValue(":description", ui->lineEdit->text());
  query_->bindValue(":friend_name", current_row_fio_);
  if (!query_->exec()) {
    qDebug() << query_->lastError().databaseText();
    qDebug() << query_->lastError().driverText();
  }
  model_->submitAll();
  setTotalInfo();
  ui->lineEdit->clear();
  emit on_calendarWidget_clicked(current_birthday_date_);
}

void Calendar::log_out() {
  settings_->clear();
  this->close();
}

void Calendar::slotInfo() {
  HelpInformation *form = new HelpInformation();
  form->setWindowModality(Qt::ApplicationModal);
  form->show();
}

void Calendar::slotAbout() {
  QMessageBox::about(this, "О программе",
                     "Версия: 0.0.1 Alpha\n\nРазработчик: Раймгужинов Диас, "
                     "ИП-113\n\n            "
                     "© 2023 уч.год, СибГУТИ");
}

void Calendar::slotShortcutF11() {
  if (this->isFullScreen()) {
    this->showNormal();
  } else {
    this->showFullScreen();
  }
}

void Calendar::on_searchInput_textChanged(const QString &arg1)
{
  QString userFilter = "self_user_id=" + QString::number(my_id_);
  QString friendFilter = "LOWER(friend_name) LIKE '%" + arg1.toLower() + "%'";
  QString dateFilter = "to_char(bday_date, 'DD.MM.YYYY') LIKE '%" + arg1 + "%'";
  if (!arg1.isEmpty()) {
    if (arg1.front().isDigit()) {
      model_->setFilter(userFilter + " AND " + dateFilter);
    } else {
      model_->setFilter(userFilter + " AND " + friendFilter);
    }
  } else {
    model_->setFilter(userFilter);
  }
}
