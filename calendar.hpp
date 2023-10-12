#pragma once

#include <QMainWindow>
#include <string>

QT_BEGIN_NAMESPACE
namespace Ui { class Calendar; }
QT_END_NAMESPACE

class Calendar : public QMainWindow
{
    Q_OBJECT

public:
    Calendar(QWidget *parent = nullptr);
    ~Calendar();
    void setDockerPath(std::string docker_args_down);

private:
    Ui::Calendar *ui;
    std::string docker_path_ = "error: there was no connection closure";
};
