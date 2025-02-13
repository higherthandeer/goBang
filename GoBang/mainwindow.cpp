#include "mainwindow.h"
#include "ui_mainwindow.h"

unsigned WINAPI connect_thread(void *arg);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(600,600);
    setWindowTitle("五子棋网络对战");
    this->setWindowIcon(QPixmap(":/res/Title1.png"));
    // 创建客户端
    client = new internet();
    // 初始页面1
    ui -> stackedWidget ->setCurrentIndex(0);

    tableModel = new QStandardItemModel(this);
    ui->tableView->setModel(tableModel);

    columnTitles << "房间信息" << "IP地址" <<"套接字"<<"进入房间";
    tableModel->setHorizontalHeaderLabels(columnTitles);

    ui->tableView->setColumnWidth(0,265);
    ui->tableView->setColumnWidth(1,100);
    ui->tableView->setColumnWidth(2,50);
    ui->tableView->setColumnWidth(3,130);

    connect(ui->updateServer, &QAction::triggered, this, &MainWindow::updateServerActionSlot);
    connect(ui->contactAuthor, &QAction::triggered, this, &MainWindow::contactAuthorActionSlot);

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::timerEvent(QTimerEvent *event)
{
    if(client->connect_thread_running)
    {
          ui->connectStatLabel->setStyleSheet("QLabel{""color:blue;""}");
          ui->connectStatLabel->setText("<正在连接服务器...>");
          return;
    }
    else if(client->get_connecting())
    {
        ui->refreshBtn->setDisabled(false);
        ui->createBtn->setDisabled(false);
        ui->connectStatLabel->setStyleSheet("QLabel{""color:green;""}");
        ui->connectStatLabel->setText("已连接服务器-<创建或者加入对局>");
        return;
    }
    else
    {
        ui->connectStatLabel->setStyleSheet("QLabel{""color:red;""}");
        ui->connectStatLabel->setText("<请连接服务器>");
        ui->refreshBtn->setDisabled(true);
        ui->createBtn->setDisabled(true);
        return ;
    }

}


void MainWindow::on_quitBtn_clicked()
{
    close();
}

void MainWindow::on_localBtn_clicked()
{
    gobang *gobang_local = new gobang();
    gobang_local -> show();
    this -> hide();

}

void MainWindow::on_internetBtn_clicked()
{
    timerId=startTimer(500);
    on_reconnectBtn_clicked();
////    Sleep(100);
////    if(client->connecting())
////        on_Button_refresh_clicked();
    ui->connectStatLabel->setStyleSheet("QLabel{""color:red;""}");
    ui->roomNameLabel->setStyleSheet("QLabel{""color:orange;""}");
    ui->connectStatLabel->setText("<请连接服务器>");
    ui->stackedWidget->setCurrentIndex(1);
    ui->refreshBtn->setDisabled(true);
    ui->createBtn->setDisabled(true);

}


void MainWindow::on_exitNetBtn_clicked()
{
    tableModel->removeRows(0,tableModel->rowCount());
    killTimer(timerId);
    if(client->connect_thread_running)
        client->connect_thread_running=false;
    if(client->get_connecting())
        client->disconnect();
    ui->stackedWidget->setCurrentIndex(0);

}

void MainWindow::on_reconnectBtn_clicked()
{
    if(client->get_connecting())
        return ;
    HANDLE thread=(HANDLE)_beginthreadex(NULL,0,connect_thread,(void*)client,0,NULL);
}

unsigned WINAPI connect_thread(void *arg)
{
    internet *client=(internet*)arg;
    qDebug()<<"线程开始"<<endl;
    client->myconnect();
    qDebug()<<"线程结束"<<endl;
}

void MainWindow::on_createBtn_clicked()
{
    if(!client->get_connecting())
            return;
        QString room_name="C:"+ui->lineEdit->text();
        int ret=client->send_msg(room_name);
        if(ret==SOCKET_ERROR)
        {
            QMessageBox::information(this,"网络对战","房间创建失败,请重试",QMessageBox::Ok);
            return ;
        }
        client->queue_clear();
        gobang_internet *gobang_internet_game=new gobang_internet(this->x(),this->y(),client,ui->lineEdit->text());
        gobang_internet_game->show();
        this->hide();
        connect(gobang_internet_game,&gobang_internet::game_over,this,[=](){qDebug()<<"www"<<endl;delete gobang_internet_game;this->show();});

}

void MainWindow::on_refreshBtn_clicked()
{
    client->queue_clear();
    if(!client->get_connecting())
        return ;
    client->send_msg("R");  // R refresh刷新消息
    Sleep(300);

    // 从消息队列中取出当前在线人数消息
    int people=client->get_msg().toInt();
    ui->peopleLabel->setText(QString("在线人数:%1").arg(people));

    // 取出下一条消息，房间总数，并转换为int
    string str=client->get_msg().toStdString();
    int sum=0;
    for(int i=0;i<str.size();i++)
        sum=sum*10+str[i]-'0';

    // 移除表格中所有行（清空数据）
    tableModel->removeRows(0,tableModel->rowCount());

    for(int i=0;i<sum;i++)
    {
        QString get;

        // 对每个房间取出name，IP，Fd
        QPushButton *button = new QPushButton("Join");

        get=client->get_msg();
        tableModel->setItem(i, 0, new QStandardItem(get));
        button->setProperty("Name",get);

        get=client->get_msg();
        tableModel->setItem(i, 1, new QStandardItem(get));
        button->setProperty("Ip",get);

        get=client->get_msg();
        tableModel->setItem(i, 2, new QStandardItem(get));
        button->setProperty("Fd",get);

        ui->tableView->setIndexWidget(tableModel->index(i, 3), button);
        connect(button,&QPushButton::pressed,this,&MainWindow::join_game);
    }

   // 根据房间数设置表格1至4列宽度
    if(sum==0)
        ui->tableView->setColumnWidth(0,265);
    else
        ui->tableView->setColumnWidth(0,250);

    ui->tableView->setColumnWidth(1,100);
    ui->tableView->setColumnWidth(2,50);
    ui->tableView->setColumnWidth(3,130);

    // 再次清空队列用于处理下一次请求
    client->queue_clear();
}

void MainWindow::join_game()
{
    client->queue_clear();
    QPushButton *button = (QPushButton *)sender();
    QString msg="J"+button->property("Fd").toString();
    if(!client->get_connecting())
            return;
    int ret=client->send_msg(msg);
    Sleep(300);
    if(ret==SOCKET_ERROR||client->queue_empty()||client->get_msg()!="success")
    {
        QMessageBox::information(this,"网络对战","房间加入失败,请重试",QMessageBox::Ok);
        on_refreshBtn_clicked();
        return ;
    }
    client->queue_clear();
    qDebug()<<"加入房间啦:"<<button->property("Fd").toString()<<endl;
    tableModel->removeRows(0,tableModel->rowCount());
    gobang_internet *gobang_internet_game=new gobang_internet(this->x(),this->y(),client,button->property("Name").toString());
    gobang_internet_game->show();
    this->hide();
    connect(gobang_internet_game,&gobang_internet::game_over,this,[=](){delete gobang_internet_game;on_refreshBtn_clicked();this->show();});
}

void MainWindow::updateServerActionSlot(){
    updateServerDialog *dialog = new updateServerDialog();
    dialog->show();
    if(dialog->exec() == QDialog::Accepted){
        QString ip = dialog -> get_ip();
        qDebug() << "ip:" << ip <<endl;
        QString port = dialog -> get_port();
        qDebug() << "port:" << port <<endl;
        // 将ip和port传递给client对象
        client -> set_addr(ip);
        client -> set_port(port);
    }

}

void MainWindow::contactAuthorActionSlot(){
    QMessageBox::information(this,"STAFF","制作者:higher than deer\nQQ:1359058611",QMessageBox::Ok);

}

