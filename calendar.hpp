#pragma once

#include <QMainWindow>
#include <QMessageBox>
#include <QOAuth2AuthorizationCodeFlow>
#include <QSettings>
#include <QShortcut>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlRelationalTableModel>
#include <QTimer>

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
    void setPhotos(QVariantList &photos);
    void setOauth(QOAuth2AuthorizationCodeFlow *oauth);

public slots:
    void loadImage(const QString &urlString);
    void setImage(const QImage &image);
    void resetImage();
    void slotInfo();
    void slotAbout();
    void slotShortcutF11();

private slots:
    void on_tableView_clicked(const QModelIndex &index);
    void on_sign_inButton_clicked();
    void on_calendarWidget_clicked(const QDate &date);
    void on_submitButton_clicked();
    void on_lineEdit_returnPressed();
    void log_out();
    void updateImages();
    void setFriendsInfo();
    void setMYInfo();
    void setTotalInfo();
    void getDate(const QDate &startDate, const QDate &endDate);
    void clearDateTextFormats();
    void showDateRange(const QDate &startDate, const QDate &endDate);
    void onImageRead(const QUrl &imageUrl, const QImage &image);
    void on_searchInput_textChanged(const QString &arg1);

private:
    Ui::Calendar *ui;
    QSettings *settings_;
    std::string docker_path_ = "echo error: there was no connection closure";
    QSqlDatabase db_;
    QSqlQuery *query_;
    QSqlRelationalTableModel *model_;
    QString current_row_fio_;
    QDate current_birthday_date_;
    QVariantList bdates_, fios_, ids_, my_ids_, photos_ = {};
    QImage currentImage_;
    qint64 my_id_ = 0;
    QString my_fio_ = "";
    QOAuth2AuthorizationCodeFlow *oauth_;
    QShortcut *keyF11;
    QShortcut *keyCtrlL;
    QShortcut *keyCtrlO;
    QShortcut *keyCtrlI;
    QShortcut *keyCtrlB;
};
