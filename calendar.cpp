#include "calendar.hpp"
#include "./ui_calendar.h"

Calendar::Calendar(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Calendar)
{
    ui->setupUi(this);
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
