#ifndef AUTHORDIALOG_H
#define AUTHORDIALOG_H

#include <QDialog>

namespace Ui {
class authorDialog;
}

class authorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit authorDialog(QWidget *parent = nullptr);
    ~authorDialog();

private:
    Ui::authorDialog *ui;
};

#endif // AUTHORDIALOG_H
