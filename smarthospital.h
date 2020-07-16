#ifndef SMARTHOSPITAL_H
#define SMARTHOSPITAL_H

#include <QMainWindow>
#include<QDebug>
#include<QVector>
#include<QList>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QMediaPlaylist>
#include <QtCharts>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QBarSeries>
#include <QBarSet>
#include<QLineSeries>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDateTime>
#include"recommend.h"
#include"ui_recommend.h"
QT_CHARTS_USE_NAMESPACE
namespace Ui {
class SmartHospital;
}

class SmartHospital : public QMainWindow
{
    Q_OBJECT

public:
    explicit SmartHospital(QWidget *parent = nullptr);
    ~SmartHospital();
    void UpdateParking();
    void Updatesql();
    void Update_barchart();
    void Update_ringchart();
    void init_barchart();
    void init_linechart();
    void init_tcpserver();

public slots:
    void new_client();
    void read_client_data();
    void client_dis();

private slots:


    void on_ExitButton_clicked();
    void on_BG_Clicked(int index);


private:
    Ui::SmartHospital *ui;
    QVector<int> ParkingStaus;
    QStringList ParkingPartition;
    QMediaPlayer *_oMediaPlayer,*_oMediaPlayer1,*_oMediaPlayer2,*_oMediaPlayer3;
    QMediaPlaylist *_pMediaPlaylist;
    QChart*                         m_barChart;
    QChart*                         m_barChart2;
    QBarSeries*                     m_barSeries;
    QBarSeries*                     m_barSeries2;
    QTcpServer *mServer;
    QTcpSocket *mSocket;
    recommend *rec;

};

#endif // SMARTHOSPITAL_H
