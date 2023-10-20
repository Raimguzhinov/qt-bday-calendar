#pragma once

#include <QMainWindow>
#include <QMessageBox>
#include <QOAuth2AuthorizationCodeFlow>
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
    void setIDs(QVariantList &ids);
    void setBDays(QVariantList &bdates);
    void setFIOs(QVariantList &fios);
    void setMYID(qint64 &my_id);
    void setMYFIO(QString &my_fio);
    void setOauth(QOAuth2AuthorizationCodeFlow *oauth);

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_tableView_clicked(const QModelIndex &index);
    void on_pushButton_6_clicked();

private:
    Ui::Calendar *ui;
    std::string docker_path_ = "error: there was no connection closure";
    QSqlDatabase db_;
    QSqlQuery *query_;
    QSqlRelationalTableModel *model_;
    size_t current_row_index_;
    QVariantList bdates_, fios_, ids_ = {};
    qint64 my_id_ = 0;
    QString my_fio_ = "";
    QOAuth2AuthorizationCodeFlow *oauth_;
    void setFriendsInfo();
    void setMYInfo();
    void setTotalInfo();
};
