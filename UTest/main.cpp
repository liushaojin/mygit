#include "utestdlg.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    UTestDlg w;
    w.show();

    return a.exec();
}
