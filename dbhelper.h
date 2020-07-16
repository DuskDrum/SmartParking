#ifndef DBHELPER_H
#define DBHELPER_H
#include <QSqlDatabase>
#include<QMessageBox>
class DBHelper
{
public:
    static DBHelper* getInstance();
    void createConn();
    void destoryConn();
private:
    QSqlDatabase benji;
    static DBHelper* instance;
    DBHelper();
};
#endif // DBHELPER_H
