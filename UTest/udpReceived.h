#ifndef UDPTRECEIVED_H
#define UDPTRECEIVED_H


#include <QThread>
#include <QMutex>
#include <QTimer>
#include <iostream>
#include <vector>
#include "udpthread.h"
#include <stdio.h>
#include <string.h>

#define RecvBufSize 1024

using namespace std;

typedef enum{
    NonePacket,                 //None
    RegisterPackage = 0x31,     //register
    AckSuccessPackage,          //ack successed
    AckFailedPackage,           //ack failed
    LinkTestPackage,            //link test
    LinkAckPackage,             //link ack
    DataPackege,                //data
    ForwardPackage,             //forward
    StopPackage,                //stop
    BackwardPackage,            //backward
    UpgradePackage,             //upgrade
}UdpPacketType;


typedef struct _udpDataPackage
{
    _udpDataPackage()
    {
        init();
    }

    unsigned char headerStx;
    unsigned char packetType;
    unsigned char messageSn;
    int dataLen;
    vector<unsigned char> dataVec;
    unsigned char headerEnd;

    void init()
    {
        headerStx = 0xeb;
        packetType = 0x00;
        messageSn = 0x00;
        dataLen = 0x00;
        dataVec.clear();
        headerEnd = 0x03;
    }

//    bool bindData(unsigned char *data, int len, UdpPacketType type)
//    {
//        if(messageSn++ > 255)
//        {
//            messageSn = 0;
//        }
//        packetType = (int)type & 0xff;
//        dataLen = len + 8;
//        int i = 0;
//        if(data == NULL)
//            return false;
//        while(i < len)
//        {
//            dataVec.push_back(data[i]);
//        }

//        return true;
//    }
}UdpDataPackage;

class UdpReceived : public QThread
{
    Q_OBJECT
public:
    explicit UdpReceived(QObject *parent = 0);
    ~UdpReceived();

    void Stop();
    void SendData(char data[], int len, UdpPacketType type);
    void SendData(char data[], int len, UdpPacketType type, bool ack);

protected:
    void run();

private:
    int GetParameterInterval();

    void DecodeRecvData(char dat[], int len);
    void DecodeRecvData(QByteArray byte, int len);
    bool MobileRegisterHandle();
    bool MobileStartFHandle();
    bool MobileStopHandle();
    bool MobileStartBHandle();
    bool MobileUpgradeHandle();

    int HexString2Int(char *c);
    int HexChar2Int(char c);
    int HexStringToInt(QString HexString);
    QString IntToHexString(QByteArray data);
    QByteArray HexStringToByteArray(QString HexString);
    QString ByteArrayToHexString(QByteArray data);

    volatile bool m_bStoped;
    QMutex m_mutex;
    UdpDataPackage m_udpDataPackage;

    int m_sendSn;
    int m_socketFd;
    int m_len;
    int m_linkTestCnt;
    bool m_isLinkTesting;
    QTimer *m_linkTimer;
    UdpThread *m_udpThread;

    struct sockaddr_in m_serverAddr;  //服务器网络地址结构体
    struct sockaddr_in m_clientAddr; //客户端网络地址结构体

    char *m_buf;
    char *m_sendBuf;
//    char m_buf[RecvBufSize];
//    char m_sendBuf[RecvBufSize];

signals:
    void UpdateConnectNum(int);
    void DataComming();

public slots:
    void OnLinkTimer();
    void OnDataComing(int);


};

#endif // UDPTRECEIVED_H
