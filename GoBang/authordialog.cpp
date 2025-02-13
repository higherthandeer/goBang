#include "authordialog.h"
#include "ui_authordialog.h"

authorDialog::authorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::authorDialog)
{
    ui->setupUi(this);
}

authorDialog::~authorDialog()
{
    delete ui;
}
