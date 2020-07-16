#include "dbhelper.h"
#include<QDebug>
#include<QSqlError>
DBHelper* DBHelper::instance = 0;

DBHelper* DBHelper::getInstance()
{
    if(instance == 0){
        instance = new DBHelper();
    }
    return instance;
}

DBHelper::DBHelper()
{
    benji = QSqlDatabase::addDatabase("QMYSQL","benji");

}

void DBHelper::createConn()
{
    QSqlDatabase benji = QSqlDatabase::addDatabase("QMYSQL","benji");
       benji.setHostName("127.0.0.1");
        //benji.setHostName(localhost);
        benji.setPort(3306);
        benji.setDatabaseName("Parking");
        benji.setUserName("root");
        benji.setPassword("123456");
        if(!benji.open())
        {
           QMessageBox::critical(0,"连接失败","无法连接中控数据库", QMessageBox::Cancel);
           qDebug()<<benji.lastError().text();
        }

}
void DBHelper::destoryConn()
{
    benji.close();
    benji.removeDatabase("benji");
}
