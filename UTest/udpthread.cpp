#include "udpthread.h"
#include <QMutexLocker>
#include <QMutex>
//#include <QReadWriteLock>

UdpThread::UdpThread(QObject *parent) :
    QThread(parent),m_bStoped(false), m_socketFd(0),
    m_len(0)
{
    InitUdpServer();
}

UdpThread::~UdpThread()
{
    if(!m_bStoped)
    {
        m_bStoped=true;
        wait(2000);
    }
}

void UdpThread::InitUdpServer()
{
    memset(&m_serverAddr,0,sizeof(m_serverAddr)); //数据初始化--清零
    m_serverAddr.sin_family=AF_INET;        //设置为IP通信
    m_serverAddr.sin_addr.s_addr=INADDR_ANY;//服务器IP地址--允许连接到所有本地地址上
    //    m_serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); //IP地址，需要进行网络序转换，INADDR_ANY：本地地址
    m_serverAddr.sin_port=htons(8000);      //服务器端口号
}

void UdpThread::Stop()
{
    m_bStoped=true;
}

void UdpThread::run()
{
    //创建服务器端套接字--IPv4协议，面向无连接通信，UDP协议
    if((m_socketFd=socket(PF_INET,SOCK_DGRAM,0))<0)
    {
        perror("socket error");
    }

    //将套接字绑定到服务器的网络地址上
    if (bind(m_socketFd,(struct sockaddr *)&m_serverAddr,sizeof(struct sockaddr))<0)
    {
        perror("bind error");
    }

    socklen_t sinSize=sizeof(struct sockaddr_in);

    while(!m_bStoped)
    {
        //接收客户端的数据并将其发送给客户端--recvfrom是无连接的
        if((m_len=recvfrom(m_socketFd, m_buf, RecvBufSize, 0, (struct sockaddr *)&m_clientAddr, &sinSize))<0)
        {
            perror("recvfrom error");
        }
        else
        {
            emit DataComming(m_len);
        }

        usleep(10000);
    }

    close(m_socketFd);  //关闭套接字
}

char *UdpThread::ReadAll()
{
    return m_buf;
}

QByteArray UdpThread::ReadAllByByte()
{
    QByteArray byte;
    byte = QByteArray(m_buf);
    return byte;
}

void UdpThread::SendData(char data[], int len)
{
    QMutexLocker locker(&m_mutex);
//    QReadWriteLock lock;
//    lock.lockForRead();

    if(data == NULL || len == 0)
    {
        return;     //no data to send
    }

    sendto(m_socketFd, data, len, 0, (struct sockaddr *)&m_clientAddr, sizeof(struct sockaddr));
}

