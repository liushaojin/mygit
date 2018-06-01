#include "utestdlg.h"
#include "ui_utestdlg.h"

#include <QHostAddress>
#include <QHostInfo>
#include <QNetworkInterface>

UTestDlg::UTestDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UTestDlg)
{
    ui->setupUi(this);
    m_udpReceive = new UdpReceived();

    ui->m_ipLineEdit->setText(GetIP());
    ui->m_portLineEdit->setText("8000");
    //ui->m_ipLineEdit->setEnabled(false);
    //ui->m_portLineEdit->setEnabled(false);
    ui->m_ipLineEdit->setFocusPolicy(Qt::NoFocus);
    ui->m_portLineEdit->setFocusPolicy(Qt::NoFocus);
    ui->m_connectEdit->setFocusPolicy(Qt::NoFocus);

    connect(m_udpReceive, SIGNAL(UpdateConnectNum(int)), this, SLOT(OnUpdateConnectNum(int)));
}

UTestDlg::~UTestDlg()
{
    delete ui;
}

void UTestDlg::OnUpdateConnectNum(int num)
{
    ui->m_connectEdit->setText(QString::number(num));
}

void UTestDlg::on_m_btnConnect_clicked()
{
    m_udpReceive->start();
    ui->m_infoListWidget->addItem(tr("Start udp server."));
}

void UTestDlg::on_m_btnDisconnect_clicked()
{
    m_udpReceive->Stop();
    ui->m_infoListWidget->addItem(tr("Stop udp server."));
}

//获取本机IP
QString UTestDlg::GetIP()
{
    QString ipAddr;
#if 1
    QList<QHostAddress> addressList = QNetworkInterface::allAddresses();
    foreach(QHostAddress address, addressList)
    {
        if(address.protocol() == QAbstractSocket::IPv4Protocol &&
                address != QHostAddress::Null &&
                address != QHostAddress::LocalHost)
        {
            if (address.toString().contains("127.0."))
            {
                continue;
            }
            ipAddr = address.toString();
            break;
        }
    }
#else
    QString lacalHost = QHostInfo::localHostName();
    QHostInfo info = QHostInfo::fromName(localHost);
    info.addresses();//QHostInfo的address函数获取本机ip地址
    //如果存在多条ip地址ipv4和ipv6：
    foreach(QHostAddress address,info.addresses())
    {
        if(address.protocol()==QAbstractSocket::IPv4Protocol){//只取ipv4协议的地址
            qDebug()<<address.toString();
            ipAddr = address.toString();
        }
    }
#endif
    return ipAddr;
}
