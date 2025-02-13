#include "gobang_internet.h"
#include "ui_gobang_internet.h"


#define BOARDSIZE 15
/*
 * 客户端向服务器发送信号：
 * R：refresh，刷新战局
 * C：create，创建房间
 * E：exit，退出房间
 * J：join，加入房间
 * U：update，更新对手准备状态
 *
*/
gobang_internet::gobang_internet(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::gobang_internet)
{
    ui->setupUi(this);
}
// 自定义构造函数
gobang_internet::gobang_internet(int x,int y,internet *client_,QString room_name,QWidget *parent):client(client_), QMainWindow(parent),ui(new Ui::gobang_internet)
{

    // 初始化ui界面并隐藏
    ui->setupUi(this);
    this->setFixedSize(1000,800);  // 固定窗口大小
    this->move(x,y);  // 将窗口移动到(x,y)位置
    ui->roomNameLabel->setText(room_name); // 设置窗口标题为房间名称
    setWindowTitle("五子棋网络对战");

    ui->stackedWidget->setCurrentIndex(0); // 第0页
    ui->blackBtn->hide();
    ui->whiteBtn->hide();
    ui->blackBtn->setStyleSheet("QPushButton{""color:white;""border-image:url(:/res/kuro.png);""}");
    ui->whiteBtn->setStyleSheet("QPushButton{""border-image:url(:/res/shiro.png);""}");

    this->setWindowIcon(QPixmap(":/res/Title1.png"));
    white_player.load(":/res/shiro.png");
    black_player.load(":/res/kuro.png");
    background.load(":/res/BackGround1.png");
    ui->prepareLabel->setStyleSheet("QLabel{color:rgba(0,0,0,0.3);}");
    ui->opponentLabel->setStyleSheet("QLabel{""border-image:url(:/res/opponent_no.png);""}");
    ui->prepareStateLabel->setStyleSheet("QLabel{""color:red;""}");
    ui->victoryLabel->setStyleSheet("QLabel{""color:red;""}");
    ui->waitLabel->setStyleSheet("QLabel{""color:red;""}");
    ui->answerLabel->setStyleSheet("QLabel{""color:yellow;""}");
    ui->victoryLabel->hide();
    ui->playerLabel->hide();
    ui->agreeBtn->hide();
    ui->refuseBtn->hide();
    ui->waitLabel->hide();
    ui->answerLabel->hide();
    ui->msgLabel->hide();

    // 初始化游戏状态
    wait=false;
    ban_mouse=false;
    isprepare=false;
    color=-1;
    running=false;
    turn=false;
    timerId1=startTimer(500);

    back.resize(0);
    square=800/(BOARDSIZE+1);

    // 初始化棋盘
    for(int i=0;i<BOARDSIZE;i++)
    {
        press_board.push_back(QVector<QPair<QRect,int>>());
        for(int j=0;j<BOARDSIZE;j++)
            press_board[i].push_back(QPair<QRect,int>(QRect((i+1)*square-square/3,(j+1)*square-square/3,square/3*2.5,square/3*2.5),-1));
    }
}

gobang_internet::~gobang_internet()
{
    qDebug()<<"网络游戏析构函数触发"<<endl;
    client -> send_msg("E"); // exit,表退出
    delete ui;
}

// 初始化
void gobang_internet::init()
{
    back.resize(0);
    ui -> regretBtn -> setDisabled(false); // 才开始没有下棋无法悔棋
    // 新游戏棋盘无棋子置为-1
    for(auto &x:press_board)
    {
        for(auto &y:x)
            y.second=-1;
    }
    update();
}


// 处理下棋函数
void gobang_internet::take_chess(int x, int y)
{
    if(!running)return;
    if(!turn)return;
    qDebug()<<"落子坐标"<<x<<" "<<y<<endl;
    QRect R(x,y,5,5);//点击的触摸大小
    //核心是判断是否点击在周围的格子，这里是一个难点*
    for(int i=0;i<press_board.size();i++)
    {
        for(int j=0;j<press_board[i].size();j++)
        {
            if(press_board[i][j].first.intersects(R)&&press_board[i][j].second==-1)
            {
                //qDebug()<<(i+1)*50<<" "<<(j+1)*50<<endl;
                press_board[i][j].second=color;
                back.push(QPair<int,int>(i,j));
                //qDebug()<<i<<" "<<j<<endl;
                QString msg="OM00";
                switch(i) // 发送消息当棋盘位置大于等于10时从a开始
                {
                    case 10:msg[2]='a';break;
                    case 11:msg[2]='b';break;
                    case 12:msg[2]='c';break;
                    case 13:msg[2]='d';break;
                    case 14:msg[2]='e';break;
                default:msg[2]=i+'0';
                }
                switch(j)
                {
                    case 10:msg[3]='a';break;
                    case 11:msg[3]='b';break;
                    case 12:msg[3]='c';break;
                    case 13:msg[3]='d';break;
                    case 14:msg[3]='e';break;
                default:msg[3]=j+'0';
                }
                client->send_msg(msg);
                check_win(i,j);
            }
        }
    }

    update();
}

// 检查输赢
void gobang_internet::check_win(char x, char y){
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

    bool black=turn?color:!color;
    for(int i = 0; i <4; i++){   // 检查四个方向上是否五颗子
        int sum = 1;
        int tx = x, ty = y;
        for(int j = 0; j < 5; j++){  // 一个方向看五次
            tx += pos1[i][0];
            ty += pos1[i][1];
            // 1.判断是否超出棋盘范围
            // 2.此处与单机时不同，玩家切换发送信息后由别的函数决定，因此此处仍然时当前玩家，玩家下棋后并不在此处切换
            if(tx>=0&&ty>=0&&tx<press_board.size()&&ty<press_board.size()&&press_board[tx][ty].second==black)
                sum++;
            else
                break;
        }
        tx = x, ty = y;
        for(int j = 0; j < 5; j++){ // 一颗棋子下后左右方向都检查因此设置pos1,pos2两层循环
            tx += pos2[i][0];
            ty += pos2[i][1];
            // 1.判断是否超出棋盘范围
            // 2.由于当前玩家p1下棋后马上切换为p2因此每次下棋判断五颗子时当前玩家为p2即!black，且若满足赢家为black
            if(tx>=0&&ty>=0&&tx<press_board.size()&&ty<press_board.size()&&press_board[tx][ty].second==black)
                sum++;
            else
                break;
        }
        if(sum >= 5){  // 五子行
            ui -> prepareLabel -> show();
            if(black){
                ui -> victoryLabel -> setText("黑棋胜利");
            }else{
                ui -> victoryLabel -> setText("白棋胜利");
            }
            on_prepareBtn_clicked();
            ui -> prepareLabel -> show();
            ui -> victoryLabel -> show();
            ui->stackedWidget->setCurrentIndex(0);
            color = -1;
            running = false; //游戏结束
            return;
        }
    }
    turn = !turn;
}

// 悔棋
void gobang_internet::go_back()
{
    if(back.empty())return;
    int x=back.top().first;
    int y=back.top().second;
    back.pop();
    press_board[x][y].second=-1;
    turn=!turn;
    update();
}

// 获取准备信息
void gobang_internet::get_prepare_information()
{
    QString msg;
    client->queue_clear();  // 清空队列消息
    client->send_msg("U");  // 发送更新消息update

    while(client -> queue_size()<4&&client->get_connecting()); // 队列大小小于4一直获取

    msg = client -> get_msg(); // msg=1表示对手已准备， 否则未准备。

    msg == "1" ? ui->opponentLabel -> setStyleSheet("QLabel{""border-image:url(:/res/opponent_ok.png);""}"):
                 ui->opponentLabel -> setStyleSheet("QLabel{""border-image:url(:/res/opponent_no.png);""}");

    msg=client->get_msg();  // 继续获取下一条信息，为1设置文字已准备，否则未准备

    msg=="1"?ui->prepareStateLabel->setText("已准备"):ui->prepareStateLabel->setText("未准备");
    msg=="1"?ui->prepareStateLabel->setStyleSheet("QLabel{""color:green;""}"):  // 设置文字颜色
             ui->prepareStateLabel->setStyleSheet("QLabel{""color:red;""}");

    // 获取下一条信息IP并设置
    msg=QString("IP:%1").arg(client->get_msg());
    ui->ipLabel->setText(msg);

    // 获取下一条信息套接字并设置
    msg=QString("FD:%1").arg(client->get_msg());
    ui->fdLabel->setText(msg);

    // 更新
    update();
}

// 等待
void gobang_internet::wait_over()
{
    wait=false;
    ui->regretBtn->setDisabled(false);
    ui->prepareLabel->hide();
    ui->waitLabel->hide();
    ui->victoryLabel->hide();
    ui->agreeBtn->hide();
    ui->refuseBtn->hide();
    ban_mouse=false;
    //ui->label_prepare->setText("等待对方响应悔棋请求...");
}

// 关闭事件
void gobang_internet::closeEvent(QCloseEvent *event)
{
    qDebug()<<"网络游戏触发关闭事件啦"<<endl;
    if(running)
    {
        client->send_msg("OR");
    }
    emit game_over();
}

// 绘制棋盘等
void gobang_internet::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPen pen;pen.setWidth(3);painter.setPen(pen);//设置棋盘线的宽度
    painter.drawPixmap(0,0,width(),height(),background);//设置背景
    int one_width=800/(BOARDSIZE+1);
    int one_height=800/(BOARDSIZE+1);
    int value=BOARDSIZE+1;
    int w=0;
    int h=0;
    for(int i=0;i<BOARDSIZE;i++)//画15条线，这里的画线写法是一个学习点*
    {
        w+=one_width;
        h+=one_height;
        //painter.drawLine(w,0,w,height());
        painter.drawLine(w,one_height,w,800/value*(value-1));
        //painter.drawLine(0,h,width(),h);
        painter.drawLine(one_width,h,800/value*(value-1),h);
    }
    pen.setWidth(8);painter.setPen(pen);
    painter.drawPoint(800/2,800/2);//中心点标注

    //以上是绘制棋盘，接下来就是绘制棋子
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
    if(running)
        if(turn)
        {
            ui->msgLabel->setText("你的回合");
            ui->msgLabel->setStyleSheet("QLabel{""color:green;""}");
        }
        else
        {
            ui->msgLabel->setText("对手回合");
            ui->msgLabel->setStyleSheet("QLabel{""color:red;""}");
        }
}

// 鼠标点击下棋
void gobang_internet::mousePressEvent(QMouseEvent *event)
{
    if(!ban_mouse)
        take_chess(event->x(),event->y());
}

// 定时器
void gobang_internet::timerEvent(QTimerEvent *event)
{
    if(!client->get_connecting())
    {
        QMessageBox::information(this,"Warnning","网络连接中断",QMessageBox::Ok);
        client->queue_clear();
        close();
        return ;
    }
    if(!running)
    {
        ui->msgLabel->hide();
        if(!client->queue_empty()&&client->get_msg()=="start")
        {
            init();
            ui->victoryLabel->hide();
            ui->stackedWidget->setCurrentIndex(1);
            ui->blackBtn->show();
            ui->whiteBtn->show();
            ui->msgLabel->show();
            ui->prepareLabel->hide();
            ui->playerLabel->hide();
            running=true;
            Sleep(300);
            return;
        }
        get_prepare_information();
    }
    else if(!wait)
    {
        if(color==-1)
        {
            while(!client->queue_empty())
            {
                QString msg=client->get_msg();
                if(msg=="c1") // 后手白棋
                {
                    color=1;
                    turn=true;
                    ui->blackBtn->hide();
                    ui->whiteBtn->hide();
                    ui->playerLabel->show();
                    ui->playerLabel->setStyleSheet("QLabel{""border-image:url(:/res/kuro.png);""}");
                }
                else if(msg=="c0") // 先手黑棋
                {
                    color=0;
                    turn=false;
                    ui->blackBtn->hide();
                    ui->whiteBtn->hide();
                    ui->playerLabel->show();
                    ui->playerLabel->setStyleSheet("QLabel{""border-image:url(:/res/shiro.png);""}");
                }
            }
        }
        else
        {
            while(!client->queue_empty())
            {
                QString recv=client->get_msg();
                string msg=recv.toStdString();
                if(msg[0]=='O')
                switch(msg[1])
                {
                    case 'M':
                    {
                        int x,y;
                        if(msg[2]>='a'&&msg[2]<='e')
                        {
                            x=msg[2]-'a'+10;
                        }
                        else
                            x=msg[2]-'0';
                        if(msg[3]>='a'&&msg[3]<='e')
                        {
                            y=msg[3]-'a'+10;
                        }
                        else
                            y=msg[3]-'0';
                        press_board[x][y].second=!color;
                        back.push(QPair<int,int>(x,y));;
                        update();
                        check_win(x,y);
                    }break;
                    case 'B':
                    {
                        ui->victoryLabel->setText("是否同意对手悔棋");
                        ui->victoryLabel->show();
                        ui->agreeBtn->show();
                        ui->refuseBtn->show();
                        ui->regretBtn->setDisabled(true);
                        ban_mouse=true;
                    }break;
                    case 'N':
                {
                    ui->LE_recv->append("[对手]:"+recv.mid(2));
                }
                    break;
                    case 'R':
                {
                    on_prepareBtn_clicked();
                    ui->prepareLabel->show();
                    QTimer *timer=new QTimer(this);
                    timer->start(5000);
                    connect(timer,&QTimer::timeout,this,[=](){ui->victoryLabel->hide();timer->stop();delete(timer);});
                    ui->victoryLabel->setText("对手离开了游戏!!!");
                    ui->victoryLabel->show();
                    ui->stackedWidget->setCurrentIndex(0);
                    color=-1;
                    running=false;
                }break;
                case 'S':
                {
                    on_prepareBtn_clicked();
                    ui->prepareLabel->show();
                    ui->victoryLabel->setText("对手认输了");
                    ui->victoryLabel->show();
                    ui->stackedWidget->setCurrentIndex(0);
                    color=-1;
                    running=false;
                    return ;
                }
                    default:break;
                }
            }
        }
    }
    else
    {
        while(!client->queue_empty())
        {
            string msg=client->get_msg().toStdString();
            qDebug()<<__LINE__<<endl;
            if(msg[0]=='O'&&msg.size()==3)
            switch(msg[1])
            {
                case 'B':
                {
                    QTimer *timer=new QTimer(this);
                    if(msg[2]=='1')
                    {
                        ui->answerLabel->setText("对手同意悔棋");
                        ui->answerLabel->show();
                        if(turn){go_back();go_back();}
                        else go_back();
                    }
                    else
                    {
                        ui->answerLabel->setText("对手不同意悔棋");
                    }
                    ui->answerLabel->show();

                    connect(timer,&QTimer::timeout,this,[=](){ui->answerLabel->hide();timer->stop();delete(timer);});
                    timer->start(3000);
                    wait_over();
                }break;
            }
        }
    }

}

// 键盘点击
void gobang_internet::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==16777220) // 按下enter键
        on_sendBtn_clicked();
}

// 准备按钮
void gobang_internet::on_prepareBtn_clicked()
{
    int ret;
    //Sleep(500);
    client->send_msg("prepare");
    if(ret==SOCKET_ERROR)
        return;
    else if(isprepare)
    {
        ui->prepareLabel->setText("请准备...");
        ui->prepareLabel->setStyleSheet("QLabel{color:rgba(255,0,0,0.3);}");
    }
    else
    {
        ui->prepareLabel->setText("请等待对方准备...");
        ui->prepareLabel->setStyleSheet("QLabel{color:rgba(0,255,0,0.3);}");
    }
    isprepare=!isprepare; // 点击按钮切换准备状态
}

void gobang_internet::on_blackBtn_clicked()
{
    if(color!=-1)
        return;
    client->send_msg("color1");

}

void gobang_internet::on_whiteBtn_clicked()
{
    if(color!=-1)
        return;
    client->send_msg("color0");
}

void gobang_internet::on_agreeBtn_clicked()
{
    client->send_msg("OB1");
    if(!turn){  // 自己下完后是对方回合对方悔棋，因此撤回两步
        go_back();
        go_back();
    }else{   // 对方下完后是自己回合，自己还没下棋时对方申请悔棋，撤回一步
        go_back();
    }
    wait_over();
}

void gobang_internet::on_refuseBtn_clicked()
{
    client->send_msg("OB0");
    wait_over();
}

// 游戏开始，退出游戏按钮
void gobang_internet::on_quitBtn_clicked()
{
    close();
}

void gobang_internet::on_loseBtn_clicked()
{
    if(!running||color==-1)
    {
        return;
    }
    client->send_msg("OS");
    on_prepareBtn_clicked();
    ui->victoryLabel->setText("你已经认输");
    ui->prepareLabel->show();
    ui->victoryLabel->show();
    ui->stackedWidget->setCurrentIndex(0);
    color=-1;
    running=false;
}

void gobang_internet::on_sendBtn_clicked()
{
    QString msg="ON"+ui->LE_send->text();
    qDebug() << msg <<endl;
    ui->LE_recv->append("[你]:"+ui->LE_send->text());
    ui->LE_send->clear();
    client->send_msg(msg);
}


// 悔棋按钮
void gobang_internet::on_regretBtn_clicked()
{
    if(!running||back.size()==0||color==-1)
    {
        return;
    }
    client->send_msg("OB");
    ban_mouse=true;
    wait=true;
    ui->regretBtn->setDisabled(true);
    ui->waitLabel->setText("等待对方响应悔棋请求");
    ui->waitLabel->show();
}

// 游戏未开始，只是进入房间退出房间按钮
void gobang_internet::on_exitRoomBtn_clicked()
{
    close();

}
