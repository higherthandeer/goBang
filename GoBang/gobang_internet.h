#ifndef GOBANG_INTERNET_H
#define GOBANG_INTERNET_H

#include <QMainWindow>
#include <QStack>
#include <QLabel>
#include <QDebug>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include"internet.h"
#include<QMouseEvent>
#include<QDebug>
#include<QMessageBox>
#include<stdio.h>
#include<stdlib.h>
#include<QTimer>

namespace Ui {
class gobang_internet;
}

class gobang_internet : public QMainWindow
{
    Q_OBJECT

public:
    explicit gobang_internet(QWidget *parent = nullptr);
    gobang_internet(int x,int y,internet *client_,QString room_name,QWidget *parent = nullptr);
    ~gobang_internet();

    internet *client;


    void init();
    void take_chess(int ,int );
    void check_win(char x,char y);//判断胜利条件
    void go_back();
    void get_prepare_information();
    void wait_over();
private:
    Ui::gobang_internet *ui;

    int square;//用于保存窗口除以格子后的平均值，便于使用。目前已经是历史遗留问题了
    QPixmap white_player; // 白棋
    QPixmap black_player; // 黑棋
    QPixmap background;  // 背景
    QStack<QPair<int,int>>back;  // 存储棋子位置？
    QVector<QVector<QPair<QRect,int>>>press_board;//核心，存储了棋盘的点击范围、当前位置是否已有子等信息。


    bool wait;  // 用于游戏运行中，一方发出悔棋、新游戏的请求后发出方持续的状态，这个状态下发出方将只等待处理对方的回应信息
    bool turn;  // 用于游戏运行中，你的回合，为你的回合时才能下棋，但此时，依然可以点击悔棋、新游戏等按钮
    int timerId1;
    int color;  // 颜色，先后手，0为白棋，1为黑棋，其他值为游戏尚未开始
    bool running;   // 游戏运行与否，为false则代表游戏处于等待状态，需要两个玩家，并且都准备
    bool isprepare; // 是否准备，0为未准备，1为准备。
    bool ban_mouse;

protected:
      void closeEvent(QCloseEvent *event);
      void paintEvent(QPaintEvent *);
      void mousePressEvent(QMouseEvent *event);
      void timerEvent(QTimerEvent* event);
      void keyPressEvent(QKeyEvent * event);
signals:
      void game_over();
private slots:
      void on_prepareBtn_clicked();
      void on_blackBtn_clicked();
      void on_whiteBtn_clicked();
      void on_agreeBtn_clicked();
      void on_refuseBtn_clicked();
      void on_quitBtn_clicked();
      void on_loseBtn_clicked();
      void on_sendBtn_clicked();
      void on_regretBtn_clicked();
      void on_exitRoomBtn_clicked();
};

#endif // GOBANG_INTERNET_H
