#ifndef INTERNER_H
#define INTERNER_H

#include<QQueue>
#include<QString>
#include<winsock2.h>
#include<stdio.h>
#include<iostream>
#include<process.h>

using namespace std;

class internet
{
public:
    internet();
    ~internet();


    bool myconnect();           // 客户端向服务端发起连接
    void disconnect();          // 客户端断开连接
    bool get_connecting();      // 获取连接状态

    bool queue_empty();         // 获取队列是否为空
    int queue_size();           // 获取信息队列大小
    SOCKET get_client_fd();     // 获取客户端描述符
    void queue_clear();         // 清空消息队列

    int send_msg(QString res);  // 向服务器发送消息
    QString get_msg();          // 获取队列中的消息
    void set_addr(QString);     // 设置IP地址
    void set_port(QString);     // 设置端口号
    void push_msg(QString res); // 将消息插入消息队列

    void msg_handle(QString);   // 处理消息
    int msg_split(int,QString); // 消息分割
    bool connect_thread_running;// 线程运行标志
private:

    WSADATA wsadata;
    SOCKADDR_IN client_addr;
    SOCKET client_fd;

    bool isconnecting;          // 判断连接状态
    bool isrecving;             // 判断接受信息状态

    QQueue<QString> msg_queue;
};

unsigned WINAPI recv_msg(void* arg);


#endif // INTERNER_H
