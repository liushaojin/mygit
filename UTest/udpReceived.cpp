#include "udpReceived.h"

#include <QDebug>


UdpReceived::UdpReceived(QObject *parent) :
    QThread(parent),m_bStoped(false), m_sendSn(0),m_socketFd(0),
    m_len(0), m_linkTestCnt(0), m_isLinkTesting(false), m_linkTimer(NULL),
    m_udpThread(NULL)
{

}

UdpReceived::~UdpReceived()
{
    if(!m_bStoped)
    {
        m_bStoped=true;
        wait(2000);
    }


    if(m_linkTimer != NULL)
    {
        if(m_linkTimer->isActive())
        {
            m_linkTimer->stop();
        }
        delete m_linkTimer;
        m_linkTimer = NULL;
    }

    if(m_udpThread != NULL)
    {
        m_udpThread->Stop();
        m_udpThread->deleteLater();
    }
}

void UdpReceived::Stop()
{
    m_bStoped=true;
}

void UdpReceived::run()
{
    m_linkTimer = new QTimer();
    connect(m_linkTimer, SIGNAL(timeout()), this, SLOT(OnLinkTimer()));
    m_linkTimer->start(1000);

    m_udpThread = new UdpThread();
    m_udpThread->start();
    connect(m_udpThread, SIGNAL(DataComming(int)), this, SLOT(OnDataComing(int)));

        //        printf("received packet from %s:/n",inet_ntoa(m_clientAddr.sin_addr));
        //        buf[m_len]='/0';
        //        printf("contents: %s/n",m_buf);

        //        if((m_len=sendto(client_sockfd,buf,strlen(buf),0,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr)))<0)
        //        {
        //            perror("recvfrom");
        //        }

        //        sprintf(m_sendBuf, "Welcome %s to here!", inet_ntoa(m_clientAddr.sin_addr));
        //        sendto(m_socketFd, m_sendBuf, strlen(m_sendBuf)+1, 0, (struct sockaddr *)&m_clientAddr, m_len);

        //usleep(10000);

    this->exec();
}

int UdpReceived::GetParameterInterval()
{
    int res = 30*1000;


    return res / 10;
}

int UdpReceived::HexChar2Int(char c)
{
    if ((c >= 'A') && (c <= 'Z'))
    {
        return c - 'A' + 10;
    }
    else if ((c >= 'a') && (c <= 'z'))
    {
        return c - 'a' + 10;
    }
    else if ((c >= '0') && (c <= '9'))
    {
        return c - '0';
    }
}

int UdpReceived::HexString2Int(char *c)
{
    int data = 0;
    int i = 0;
    if(c != NULL)
    {
        int len = strlen(c);
        while (c && (i<len))//((*c) != ' ')  ((*c != "") || (*c != " "))
        {
            data = (data << (4 * i)) | HexChar2Int(*c);
            c++;
            i++;
        }
    }
    return data;
}
//#define DecodeByChar
#ifdef DecodeByChar
void UdpReceived::OnDataComing(int len)
{
    m_buf = m_udpThread->ReadAll();

    emit UpdateConnectNum(1);
    m_linkTestCnt = 0;
    m_linkTimer->start(GetParameterInterval());//GetParameterInterval()
    DecodeRecvData(m_buf, len);
}
#else
void UdpReceived::OnDataComing(int len)
{
    QByteArray byte = m_udpThread->ReadAllByByte();

    emit UpdateConnectNum(1);
    m_linkTestCnt = 0;
    m_linkTimer->start(GetParameterInterval());//GetParameterInterval()

    DecodeRecvData(byte, len);
}
#endif
/*
void UdpReceived::OnDataComing(int len)
{
    m_buf = m_udpThread->ReadAll();
//    char *buf[len];

//    char *s = strsep(&m_buf, " ");
//    int i=0;
//    while(s)
//    {
//        buf[i] = s;
//        s = strsep(&m_buf, " ");
//        i++;
//    }

//    int idat[i];
//    for(int j=0;j<i;j++)
//    {
//        idat[j] = HexString2Int(buf[j]);
//    }

//    QByteArray byte = m_udpThread->ReadAllByByte();
//    char buf[len];
//    for(int i=0;i<len;i++)
//    {
//        char c = m_buf[i];
//        buf[i] = m_buf[i];
//    }
    emit UpdateConnectNum(1);
    m_linkTestCnt = 0;
    m_linkTimer->start(GetParameterInterval());//GetParameterInterval()
    DecodeRecvData(m_buf, len);
    //DecodeRecvData(byte, len);
}*/

void UdpReceived::DecodeRecvData(char dat[], int len)
{
    if(dat == NULL)
    {
        return;
    }

    //split string by char ' ', and reback a array of 16 decimal
    char *buf[len];

    char *s = strsep(&dat, " ");
    int i=0;
    while(s)
    {
        buf[i] = s;
        s = strsep(&dat, " ");
        i++;
    }

    int idat[i];
    for(int j=0;j<i;j++)
    {
        idat[j] = HexString2Int(buf[j]);
    }

    if(len > 7)
    {
        int headerStx = idat[0];
        int packageType = idat[1];
        //        int messageSn = idat[2];
        int datLen = (idat[6] << 24) | (idat[5] << 16) | (idat[4] << 8) | idat[3];
        int headerEnd = idat[datLen - 1];

        if(headerStx == 0xEB && datLen == i && headerEnd == 0x03)
        {
            //have received data, so should recount and reset relate flag
            m_linkTestCnt = 0;
            m_linkTimer->start(GetParameterInterval());

            switch ((UdpPacketType)packageType)
            {
            case RegisterPackage:
                if(MobileRegisterHandle())
                {
                    SendData(NULL, 0, RegisterPackage, true);
                }
                else
                {
                    SendData(NULL, 0, RegisterPackage, false);
                }
                break;
            case AckSuccessPackage:

                break;
            case AckFailedPackage:

                break;
            case LinkTestPackage:
                SendData(NULL, 0, LinkTestPackage, true);
                break;
            case LinkAckPackage:

                break;
            case DataPackege:

                break;
            case ForwardPackage:
                if(MobileStartFHandle())
                {
                    SendData(NULL, 0, ForwardPackage, true);
                }
                else
                {
                    SendData(NULL, 0, ForwardPackage, false);
                }
                break;
            case StopPackage:
                if(MobileStopHandle())
                {
                    SendData(NULL, 0, StopPackage, true);
                }
                else
                {
                    SendData(NULL, 0, StopPackage, false);
                }
                break;
            case BackwardPackage:
                if(MobileStartBHandle())
                {
                    SendData(NULL, 0, BackwardPackage, true);
                }
                else
                {
                    SendData(NULL, 0, BackwardPackage, false);
                }
                break;
            case UpgradePackage:
                if(MobileUpgradeHandle())
                {
                    SendData(NULL, 0, UpgradePackage, true);
                }
                else
                {
                    SendData(NULL, 0, UpgradePackage, false);
                }
                break;
            default:
                break;
            }
        }

    }
}

void UdpReceived::DecodeRecvData(QByteArray dat, int len)
{
    if(dat.isEmpty())
    {
        return;
    }

//    QList<QByteArray> list;
//    list = dat.split(' ');
    dat = dat.replace(" ", "");
    if(dat.length() % 2 != 0)
    {
        dat.append("");
    }
    QByteArray dataArray = QByteArray(dat.length()/2, ' ');
    int datlen = dataArray.length();
    int idat[datlen];
    for(int i=0;i<datlen;i++)
    {
        int h = (HexChar2Int(dat[i*2]) << 4);
        int l = HexChar2Int(dat[i*2+1]);
        idat[i] = h | l;
        //idat[i] = (HexChar2Int(dat[i*2]) << (4 * i)) | HexChar2Int(dat[i*2+1]);
         //dataArray[i] = (HexChar2Int(dat[i*2]) << (4 * i)) | HexChar2Int(dat[i*2+1]);
    }

    if(len > 7)
    {
        int headerStx = idat[0];
        int packageType = idat[1];
        //        char messageSn = dat[2];
        int datLen = (idat[6] << 24) | (idat[5] << 16) | (idat[4] << 8) | idat[3];
        int headerEnd = idat[datLen - 1];

        if(headerStx == 0xEB && datLen == datlen && headerEnd == 0x03)
        {
            //have received data, so should recount and reset relate flag
            m_linkTestCnt = 0;
            m_linkTimer->start(GetParameterInterval());

            switch ((UdpPacketType)packageType)
            {
            case RegisterPackage:
                if(MobileRegisterHandle())
                {
                    SendData(NULL, 0, RegisterPackage, true);
                }
                else
                {
                    SendData(NULL, 0, RegisterPackage, false);
                }
                break;
            case AckSuccessPackage:

                break;
            case AckFailedPackage:

                break;
            case LinkTestPackage:
                SendData(NULL, 0, LinkTestPackage, true);
                break;
            case LinkAckPackage:

                break;
            case DataPackege:

                break;
            case ForwardPackage:
                if(MobileStartFHandle())
                {
                    SendData(NULL, 0, ForwardPackage, true);
                }
                else
                {
                    SendData(NULL, 0, ForwardPackage, false);
                }
                break;
            case StopPackage:
                if(MobileStopHandle())
                {
                    SendData(NULL, 0, StopPackage, true);
                }
                else
                {
                    SendData(NULL, 0, StopPackage, false);
                }
                break;
            case BackwardPackage:
                if(MobileStartBHandle())
                {
                    SendData(NULL, 0, BackwardPackage, true);
                }
                else
                {
                    SendData(NULL, 0, BackwardPackage, false);
                }
                break;
            case UpgradePackage:
                if(MobileUpgradeHandle())
                {
                    SendData(NULL, 0, UpgradePackage, true);
                }
                else
                {
                    SendData(NULL, 0, UpgradePackage, false);
                }
                break;
            default:
                break;
            }
        }

    }
}

bool UdpReceived::MobileRegisterHandle()
{
    bool res = false;



    return res;
}

bool UdpReceived::MobileStartFHandle()
{
    bool res = false;



    return res;
}

bool UdpReceived::MobileStopHandle()
{
    bool res = false;



    return res;
}

bool UdpReceived::MobileStartBHandle()
{
    bool res = false;



    return res;
}

bool UdpReceived::MobileUpgradeHandle()
{
    bool res = false;



    return res;
}

void UdpReceived::OnLinkTimer()
{
    m_linkTestCnt++;

    if(m_linkTestCnt >= 10)
    {
        m_isLinkTesting = true;

        if(m_linkTestCnt >= 13)
        {
            m_linkTestCnt = 13; //link is offline
            //close this udpsocket link, Close the session
        }
        else
        {
            SendData(NULL, 0, LinkTestPackage, false);
        }
    }

}

void UdpReceived::SendData(char data[], int len, UdpPacketType type)
{
    //    if(data == NULL || len == 0)
    //    {
    //        return;     //no data to send
    //    }

    if(m_sendSn++ > 255)
    {
        m_sendSn = 0;
    }

    int datLen = len + 8;
    char dat[datLen];
    for(int i=0; i<datLen; i++)
    {
        dat[i] = 0;
    }

    dat[0] = 0xEB;
    dat[1] = (int)type & 0xff;
    dat[2] = m_sendSn;
    dat[3] = (char)datLen;
    dat[4] = (char)(datLen >> 8);
    dat[5] = (char)(datLen >> 16);
    dat[6] = (char)(datLen >> 24);

    if(data != NULL && len > 0)
    {
        for(int i = 0; i < len; i++)
        {
            dat[7 + i] = *(data+i);
        }
    }

    dat[datLen - 1] = 0x03;

    m_udpThread->SendData(dat, datLen);
//    sendto(m_socketFd, dat, datLen, 0, (struct sockaddr *)&m_clientAddr, sizeof(struct sockaddr));
//    delete[] dat;
}

void UdpReceived::SendData(char data[], int len, UdpPacketType type, bool ackRes)
{
    if(m_sendSn++ > 255)
    {
        m_sendSn = 0;
    }

    int datLen = len + 8;
    char dat[datLen];
    for(int i=0; i<datLen; i++)
    {
        dat[i] = 0;
    }

    dat[0] = 0xEB;

    if(type == LinkTestPackage)
    {
        if(ackRes)
        {
            dat[1] = (int)LinkAckPackage && 0xff;
        }
        else
        {
            dat[1] = (int)LinkTestPackage && 0xff;
        }
    }
    else
    {
        dat[1] = 0x33;

        if(ackRes)
        {
            dat[1] = 0x32;
        }
    }

    dat[2] = 0x0; //m_sendSn;
    dat[3] = (char)datLen;
    dat[4] = (char)(datLen >> 8);
    dat[5] = (char)(datLen >> 16);
    dat[6] = (char)(datLen >> 24);

    if(data != NULL && len > 0)
    {
        for(int i = 0; i < len; i++)
        {
            dat[7 + i] = *(data+i);
        }
    }

    dat[datLen - 1] = 0x03;

    m_udpThread->SendData(dat, datLen);
//    sendto(m_socketFd, dat, datLen, 0, (struct sockaddr *)&m_clientAddr, sizeof(struct sockaddr));
//    delete[] dat;
}

int UdpReceived::HexStringToInt(QString HexString)
{
    int res = 0;

    return res;
}

QString UdpReceived::IntToHexString(QByteArray data)
{
    QString resStr;


    return resStr;
}

QByteArray UdpReceived::HexStringToByteArray(QString HexString)
{
    bool ok;
    QByteArray ret;
    HexString = HexString.trimmed();
    HexString = HexString.simplified();
    QStringList sl = HexString.split(" ");

    foreach (QString s, sl) {
        if(!s.isEmpty())
        {
            char c = s.toInt(&ok,16)&0xFF;
            if(ok){
                ret.append(c);
            }else{
                qDebug()<<"非法的16进制字符："<<s;
                //QMessageBox::warning(0,tr("错误："),QString("非法的16进制字符: \"%1\"").arg(s));
            }
        }
    }
    qDebug()<<ret;
    return ret;
}

QString UdpReceived::ByteArrayToHexString(QByteArray data)
{
    QString ret(data.toHex().toUpper());
    int len = ret.length()/2;
    qDebug()<<len;
    for(int i=1;i<len;i++)
    {
        qDebug()<<i;
        ret.insert(2*i+i-1," ");
    }

    return ret;
}

