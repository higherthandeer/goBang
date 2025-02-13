#ifndef GOBANG_H
#define GOBANG_H

#include <QMainWindow>
#include <QStack>
#include <QLabel>
#include <QDebug>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
namespace Ui {
class gobang;
}

class gobang : public QMainWindow
{
    Q_OBJECT

public:
    explicit gobang(QWidget *parent = nullptr);
    ~gobang();

    void init(); // 初始化
    void check_win(int x, int y);  // 判断胜利
    void take_chess(int x, int y); // 下棋

private slots:

    void on_regretBtn_clicked();

    void on_quitBtn_clicked();

    void on_newGameBtn_clicked();

private:
    Ui::gobang *ui;

    int square;//用于保存窗口除以格子后的平均值，便于使用。目前已经是历史遗留问题了
    QPixmap white_player; // 白棋
    QPixmap black_player; // 黑棋
    QPixmap background;  // 背景
    QStack<QPair<int,int>>back;  // 存储棋子位置？
    QVector<QVector<QPair<QRect,int>>>press_board;//核心，存储了棋盘的点击范围、当前位置是否已有子等信息。
    bool black=true;  // 黑棋先手用于切换当前下棋player
    bool running;
protected:
    void paintEvent(QPaintEvent *);
//    void closeEvent(QCloseEvent *event);
    void mousePressEvent(QMouseEvent *event);
//signals:
////      void game_over();
};

#endif // GOBANG_H
