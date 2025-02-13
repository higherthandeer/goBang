#include "gobang.h"
#include "ui_gobang.h"

#define BOARDSIZE 15

gobang::gobang(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::gobang)
{
    ui->setupUi(this);
    back.resize(0);
    setWindowTitle("五子棋");
    this -> setWindowIcon(QPixmap(":/res/Title1.png"));
    white_player.load(":/res/shiro.png");
    black_player.load(":/res/kuro.png");
    background.load(":/res/BackGround1.png");
//    background.load(":/res/bg.jpg");

    square = 800/(BOARDSIZE+1);  // 棋盘格子大小，因为棋盘大小为15*15指的是交叉点，格子为16*16
    for(int i=0;i<BOARDSIZE;i++)
    {
        press_board.push_back(QVector<QPair<QRect,int>>()); // 创建第 i 行
        for(int j=0;j<BOARDSIZE;j++)
            press_board[i].push_back(QPair<QRect,int>(QRect((i+1)*square-square/3,(j+1)*square-square/3,square/3*2.5,square/3*2.5),-1));
            // 起始点坐标为（1，1），以（1，1）为中心点开始设置矩形
    }
    init();
}

gobang::~gobang()
{
    qDebug()<<"本地游戏关闭"<<endl;
    delete ui;
}

void gobang::init()
{
    ui -> backLabel -> setText("");
    ui -> backLabel -> setStyleSheet("QLabel{"
                                     "color:red;"
                                     "}");  // ???
    running = true;
    back.resize(0);
    ui -> regretBtn -> setDisabled(false); // 才开始没有下棋无法悔棋
    // 新游戏棋盘无棋子置为-1
    for(auto &x:press_board)
    {
        for(auto &y:x)
            y.second=-1;
    }
    black=true;
    update();
}

void gobang::paintEvent(QPaintEvent *){
    QPainter painter(this);
    QPen pen;
    pen.setWidth(3);
    painter.setPen(pen);
    painter.drawPixmap(0,0,width(),height(),background);//设置背景
    ui -> playerLabel -> setStyleSheet("QLabel{"
                                  "border-image:url(:/res/kuro.png);"
                                  "}");
    int one_width = 800/(BOARDSIZE+1);
    int one_height = one_width;
    int value = BOARDSIZE + 1;
    int endPos = 800 / value * (value - 1);
    int w = 0;
    int h = 0;
    // 绘制棋盘
    for(int i=0;i<BOARDSIZE;i++)//画15条线，这里的画线写法是一个学习点*
    {
        w += one_width;
        h += one_height;
        //painter.drawLine(w,0,w,height());
        painter.drawLine(w, one_height, w, endPos);    // 画竖线
        //painter.drawLine(0,h,width(),h);
        painter.drawLine(one_width, h, endPos, h);     // 画横线
    }
    pen.setWidth(8);painter.setPen(pen);
    painter.drawPoint(800/2,800/2);//中心点标注
    if(black)
        ui->playerLabel->setStyleSheet("QLabel{"
                                      "border-image:url(:/res/kuro.png);"
                                      "}");
    else
        ui->playerLabel->setStyleSheet("QLabel{"
                                      "border-image:url(:/res/shiro.png);"
                                      "}");
    // 绘制棋子
    for(auto x:press_board)
    {
        for(auto y:x)
        {
            //painter.drawRect(y.first);
            switch(y.second)
            {
                case 1:painter.drawPixmap(y.first,black_player);break;
                case 0:painter.drawPixmap(y.first,white_player);break;
                case -1:break;
                default:break;
            }
        }
    }

}

void gobang::check_win(int x, int y){
    int pos1[4][2] = {{-1, 0},   // 左
                      {0, -1},   // 上
                      {-1, -1},  // 左上
                      {-1, 1},   // 左下
                     };
    int pos2[4][2] = {{1, 0},   // 右
                      {0, 1},   // 下
                      {1, 1},  // 右下
                      {1, -1},   // 右上
                     };

    for(int i = 0; i <4; i++){   // 检查四个方向上是否五颗子
        int sum = 1;
        int tx = x, ty = y;
        for(int j = 0; j < 5; j++){  // 一个方向看五次
            tx += pos1[i][0];
            ty += pos1[i][1];
            // 1.判断是否超出棋盘范围
            // 2.由于当前玩家p1下棋后马上切换为p2因此每次下棋判断五颗子时当前玩家为p2即!black，且若满足赢家为black
            if(tx>=0&&ty>=0&&tx<press_board.size()&&ty<press_board.size()&&press_board[tx][ty].second==(!black))
                sum++;
            else
                break;
        }
        tx = x, ty = y; // 重置tx,ty，开始检查右边pos2
        for(int j = 0; j < 5; j++){
            tx += pos2[i][0];
            ty += pos2[i][1];
            // 1.判断是否超出棋盘范围
            // 2.由于当前玩家p1下棋后马上切换为p2因此每次下棋判断五颗子时当前玩家为p2即!black，且若满足赢家为black
            if(tx>=0&&ty>=0&&tx<press_board.size()&&ty<press_board.size()&&press_board[tx][ty].second==(!black))
                sum++;
            else
                break;
        }
        if(sum >= 5){  // 五子行
            QString str;
            if(black){
                str = "白棋胜利！";
            }else{
                str = "黑棋胜利！";
            }
            ui -> backLabel -> setText(str);
            ui -> regretBtn -> setDisabled(true); // 胜负已分不允许悔棋

            running = false; //游戏结束
        }
    }
}

void gobang::take_chess(int x, int y){
    if(!running) return;
    QRect R(x, y, 5, 5); // 点击的触摸大小
    // 判断点击在某个点的周围，注意判断不要点击到另一个点
    for(int i=0;i<press_board.size();i++)
    {
        for(int j=0;j<press_board[i].size();j++)
        {
            // 点击点与棋盘某个点矩形有交点且此处未放置棋子
            if(press_board[i][j].first.intersects(R)&&press_board[i][j].second==-1)
            {
                press_board[i][j].second=black; // 在棋盘上落子为当前玩家棋子颜色
                black=!black;  // 切换玩家
                back.push(QPair<int,int>(i,j));
                qDebug()<<"落子位置"<<i<<" "<<j<<endl;
                check_win(i,j);
            }
        }
    }
    // 落子重新渲染棋盘
    update();

}
//void gobang::closeEvent(QCloseEvent *event){
//    qDebug()<<"触发关闭事件啦"<<endl;
//    emit game_over();
//}

void gobang::mousePressEvent(QMouseEvent *event){
    take_chess(event->x(),event->y());
}

void gobang::on_regretBtn_clicked()
{
    if(back.empty())return;
    int x=back.top().first;
    int y=back.top().second;
    back.pop();
    press_board[x][y].second=-1;
    black=!black;
    update();
}

void gobang::on_quitBtn_clicked()
{
    close();
}

void gobang::on_newGameBtn_clicked()
{
    init();
}
