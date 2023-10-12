#include "calendar.hpp"
#include "./ui_calendar.h"

Calendar::Calendar(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Calendar)
{
    ui->setupUi(this);
}

Calendar::~Calendar()
{
    system("docker-compose down");
    delete ui;
}

