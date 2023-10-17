#pragma once

#include <QMainWindow>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlRelationalTableModel>

#include <string>

QT_BEGIN_NAMESPACE
namespace Ui { class Calendar; }
QT_END_NAMESPACE

class Calendar : public QMainWindow
{
    Q_OBJECT

public:
    Calendar(QWidget *parent = nullptr, QSqlDatabase *db = nullptr);
    ~Calendar();
    void setDockerPath(std::string docker_args_down);
    void setIDs(QVector<qint64> ids);
    void setBDays(QVector<QString> bdates);
    void setFIOs(QVector<QString> fios);
    void setMYID(qint64 my_id);
    void setMYFIO(QString my_fio);

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_tableView_clicked(const QModelIndex &index);
    void on_pushButton_6_clicked();

signals:
    void sendData(QString str);

private:
    Ui::Calendar *ui;
    std::string docker_path_ = "error: there was no connection closure";
    QSqlDatabase db_;
    QSqlQuery *query_;
    QSqlQuery *query2_;
    QSqlRelationalTableModel *model_;
    size_t current_row_index_;
    QVector<QString> bdates_, fios_ = {};
    QVector<qint64> ids_ = {};
    qint64 my_id_ = 0;
    QString my_fio_ = "";
    void setFriendsInfo();
    void setMYInfo();
    void setTotalInfo();
};
