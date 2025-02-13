#include "updateserverdialog.h"
#include "ui_updateserverdialog.h"

updateServerDialog::updateServerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::updateServerDialog)
{
    ui->setupUi(this);
}

updateServerDialog::~updateServerDialog()
{
    delete ui;
}

QString updateServerDialog::get_ip() const
{
    return ip;
}

QString updateServerDialog::get_port() const
{
    return port;
}


void updateServerDialog::on_confirmBtn_clicked()
{
    ip = ui -> IPLineEdit -> text();
    port = ui -> portLineEdit -> text();
    accept(); // 关闭 Dialog，返回给调用者

}

void updateServerDialog::on_cancelBtn_clicked()
{
    close();
}
