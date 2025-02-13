#ifndef UPDATESERVERDIALOG_H
#define UPDATESERVERDIALOG_H

#include <QDialog>

namespace Ui {
class updateServerDialog;
}

class updateServerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit updateServerDialog(QWidget *parent = nullptr);
    ~updateServerDialog();

    QString get_ip() const;
    QString get_port() const;

private slots:

    void on_confirmBtn_clicked();

    void on_cancelBtn_clicked();

private:
    Ui::updateServerDialog *ui;

    QString ip;
    QString port;
};

#endif // UPDATESERVERDIALOG_H
