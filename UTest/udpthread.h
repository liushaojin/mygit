#ifndef UDPTHREAD_H
#define UDPTHREAD_H

#include <QByteArray>
#include <QThread>
#include <QMutex>
#include <iostream>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <pthread.h>
#include <semaphore.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define RecvBufSize 1024


class UdpThread : public QThread
{
    Q_OBJECT
public:
    explicit UdpThread(QObject *parent = 0);
    ~UdpThread();

    void Stop();
    void SendData(char data[], int len);
    char *ReadAll();
    QByteArray ReadAllByByte();

protected:
    void run();

private:
    void InitUdpServer();

    volatile bool m_bStoped;
    QMutex m_mutex;

    int m_sendSn;
    int m_socketFd;
    int m_len;


    struct sockaddr_in m_serverAddr;  //服务器网络地址结构体
    struct sockaddr_in m_clientAddr; //客户端网络地址结构体
    char m_buf[RecvBufSize];
    char m_sendBuf[RecvBufSize];

signals:
    void DataComming(int);

public slots:


};

#endif // UDPTHREAD_H
