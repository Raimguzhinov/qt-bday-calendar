#include "helpinformation.hpp"
#include "ui_helpinformation.h"

HelpInformation::HelpInformation(QWidget *parent)
    : QWidget(parent), ui(new Ui::HelpInformation) {
  ui->setupUi(this);
  this->setMaximumSize(this->width(), this->height());
  this->setMinimumSize(this->width(), this->height());
  connect(ui->textBrowser, SIGNAL(anchorClicked(QUrl)), this,
          SLOT(on_textBrowser_anchorClicked(QUrl)));
  connect(ui->nextButton, SIGNAL(clicked()), ui->textBrowser, SLOT(forward()));
  connect(ui->backButton, SIGNAL(clicked()), ui->textBrowser, SLOT(backward()));
  connect(ui->homeButton, SIGNAL(clicked()), ui->textBrowser, SLOT(home()));
  connect(ui->textBrowser, SIGNAL(forwardAvailable(bool)), ui->nextButton,
          SLOT(setEnabled(bool)));
  connect(ui->textBrowser, SIGNAL(backwardAvailable(bool)), ui->backButton,
          SLOT(setEnabled(bool)));
  ui->textBrowser->setSource(
      QUrl::fromLocalFile(":/resources/helpinformation/index.html"));
}

HelpInformation::~HelpInformation() { delete ui; }
