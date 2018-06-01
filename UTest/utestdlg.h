#ifndef UTESTDLG_H
#define UTESTDLG_H

#include <QDialog>
#include "udpReceived.h"

namespace Ui {
class UTestDlg;
}

class UTestDlg : public QDialog
{
    Q_OBJECT

public:
    explicit UTestDlg(QWidget *parent = 0);
    ~UTestDlg();

private slots:
    void on_m_btnConnect_clicked();
    void on_m_btnDisconnect_clicked();
    void OnUpdateConnectNum(int);

private:
    QString GetIP();

    Ui::UTestDlg *ui;
    UdpReceived * m_udpReceive;
};

#endif // UTESTDLG_H
