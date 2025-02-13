#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QtWidgets>
#include<QDebug>
#include "gobang.h"
#include "internet.h"
#include "gobang_internet.h"
#include "updateserverdialog.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    internet *client;

    int timerId;

    QStandardItemModel *tableModel;
    QStringList columnTitles;
    //int bug1=0;//在加入其他房间游戏时，有时候会打开两个游戏，原因猜测是多次点击join按钮
    void join_game();

protected:
    void timerEvent(QTimerEvent* event);

private slots:
    void on_quitBtn_clicked();

    void on_localBtn_clicked();

    void on_internetBtn_clicked();

    void on_exitNetBtn_clicked();

    void on_reconnectBtn_clicked();

    void on_createBtn_clicked();

    void on_refreshBtn_clicked();

    void updateServerActionSlot();

    void contactAuthorActionSlot();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
