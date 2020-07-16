#include "smarthospital.h"
#include <QApplication>
#include<QDebug>
int main(int argc, char *argv[])
{
    qDebug()<<"bug";
    QApplication a(argc, argv);
    SmartHospital w;
    w.show();

    return a.exec();
}
