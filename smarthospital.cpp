#include "smarthospital.h"
#include "ui_smarthospital.h"
#include<QPixmap>
#include<QFileDialog>
#include<dbhelper.h>
#include<QSqlQuery>
#include<QSqlError>

SmartHospital::SmartHospital(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SmartHospital)
{
    ui->setupUi(this);
    DBHelper *db=DBHelper::getInstance();
    db->createConn();

    _oMediaPlayer = new QMediaPlayer(this);
    _oMediaPlayer1 = new QMediaPlayer(this);
    _oMediaPlayer2 = new QMediaPlayer(this);
    _oMediaPlayer3 = new QMediaPlayer(this);
    _pMediaPlaylist = new QMediaPlaylist(_oMediaPlayer);
    _oMediaPlayer->setVideoOutput(ui->widget);
    _oMediaPlayer1->setVideoOutput(ui->widget_2);
    _oMediaPlayer2->setVideoOutput(ui->widget_3);
    _oMediaPlayer3->setVideoOutput(ui->widget_4);
    QString path = "测试短片.mp4";
    if(path.isEmpty())
        return;
    qDebug() << __FILE__ << __LINE__ << path;
    _pMediaPlaylist->clear();
    _pMediaPlaylist->addMedia(QUrl::fromLocalFile(path));
    _pMediaPlaylist->setCurrentIndex(0);
    _pMediaPlaylist->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
    _oMediaPlayer->setPlaylist(_pMediaPlaylist);
    _oMediaPlayer1->setPlaylist(_pMediaPlaylist);
    _oMediaPlayer2->setPlaylist(_pMediaPlaylist);
    _oMediaPlayer3->setPlaylist(_pMediaPlaylist);
    _oMediaPlayer->play();
    _oMediaPlayer1->play();
    _oMediaPlayer2->play();
    _oMediaPlayer3->play();
    this-> showFullScreen();
    Updatesql();
    UpdateParking();
    init_barchart();
    Update_barchart();
    Update_ringchart();
    init_tcpserver();
    connect(ui->buttonGroup,SIGNAL(buttonClicked(int)),this,SLOT(on_BG_Clicked(int)));
    
}

SmartHospital::~SmartHospital()
{
    delete ui;
}
void SmartHospital::Updatesql()
{
    ParkingStaus.clear();
    ParkingPartition.clear();
    QString str=QString("select * from parking_real ");
    qDebug()<<str;
    QSqlDatabase benji=QSqlDatabase::database("benji");
    QSqlQuery benjiquery(benji);
    benjiquery.exec(str);
    if(benjiquery.size()!=0)
    {
        while (benjiquery.next()) {
            ParkingStaus<<benjiquery.value(1).toInt();
            ParkingPartition<<benjiquery.value(5).toString();
        }
    }
}
void SmartHospital::init_tcpserver()
{
    //初始化服务器server对象
    mServer = new QTcpServer();
    //关联客户端连接信号newConnection
    connect(mServer,SIGNAL(newConnection()),this,SLOT(new_client())); //连接客户端
    //启动服务器监听
    mServer->listen(QHostAddress::Any,9988);
}

void SmartHospital::new_client()
{
    qDebug()<<"新客户段连接";
    mSocket = mServer->nextPendingConnection();//与客户端通信的套接字
    //关联接收客户端数据信号readyRead信号（客户端有数据就会发readyRead信号）
    connect(mSocket,SIGNAL(readyRead()),this,SLOT(read_client_data()));
    //检测掉线信号
    connect(mSocket,SIGNAL(disconnected()),this,SLOT(client_dis()));
}
void SmartHospital::read_client_data()
{
    //可以实现同时读取多个客户端发送过来的消息
    QTcpSocket *obj = (QTcpSocket*)sender();
    QByteArray array = obj->readAll();
    QString msg=QString::fromLocal8Bit(array);
    QDateTime current_time =QDateTime::currentDateTime();
    QString current_datetime = current_time.toString("yyyy-MM-dd hh:mm:ss");
    QStringList list=msg.split(",");
    QString send_content,view_content;
    if(list[0]=="请求入场")
    {
        qDebug()<<"请求入场";
        view_content=current_datetime+" 收到： ";
        ui->textEdit->append(view_content);
        view_content="车辆 "+list[1]+" 请求入场";
        ui->textEdit->append(view_content);
        QString str=QString("select 车位 from parking_real where 状态='0'");
        qDebug()<<str;
        QSqlDatabase benji=QSqlDatabase::database("benji");
        QSqlQuery benjiquery(benji);
        benjiquery.exec(str);
        QString av_site;
        if(benjiquery.size()>0)
        {
           benjiquery.first();
           av_site=benjiquery.value(0).toString();
           qDebug()<<av_site;
           send_content=list[0]+","+list[1]+","+av_site;
           view_content="分配车位 "+av_site+" 给车辆 "+list[1];
           ui->textEdit->append(view_content);
           ui->textMoveWidget->setText(view_content);
           mSocket->write((send_content).toLocal8Bit());
           QString info=list[1]+"分配车位"+av_site;
           str=QString("update parking_real set 车牌='%1',状态='3',停车时间='%2' where 车位='%3'").arg(list[1]).arg(current_datetime).arg(av_site);
           qDebug()<<str;
           qDebug()<<benjiquery.exec(str);
           Updatesql();
           UpdateParking();
           init_barchart();
           Update_barchart();
           Update_ringchart();
        }
        else{
            send_content=msg+",分配失败";
            mSocket->write((send_content).toLocal8Bit());
            view_content="本停车场已满！";
            ui->textEdit->append(view_content);
            ui->textMoveWidget->setText(view_content);
        }

    }
    else if(list[0]=="智能推荐")
    {
        qDebug()<<"智能推荐";
        view_content=current_datetime+" 收到： ";
        ui->textEdit->append(view_content);
        view_content="车辆 "+list[1]+" 请求推荐停车场";
        ui->textEdit->append(view_content);
        rec=new recommend;
        QString str=QString("SELECT * FROM parking_others ORDER BY `距离（千米）`,`价格（元/小时）`");
        qDebug()<<str;
        QSqlDatabase benji=QSqlDatabase::database("benji");
        QSqlQuery benjiquery(benji);
        benjiquery.exec(str);
        QString rec_veiw;
        //QString av_site;
        if(benjiquery.size()>0)
        {
             while (benjiquery.next()) {
                 rec_veiw=benjiquery.value(0).toString()+" "+"空闲车位："+benjiquery.value(4).toString()+" "+"价格（元/小时）："+benjiquery.value(2).toString()+" "+"距离（千米）："+benjiquery.value(1).toString();
                 rec->ui->textEdit_distance->append(rec_veiw);
             }
        }
        str=QString("SELECT * FROM parking_others ORDER BY `价格（元/小时）`,`距离（千米）`");
        qDebug()<<str;
        benjiquery.exec(str);
        if(benjiquery.size()>0)
        {
             while (benjiquery.next()) {
                 rec_veiw=benjiquery.value(0).toString()+" "+"空闲车位："+benjiquery.value(4).toString()+" "+"距离（千米）："+benjiquery.value(1).toString()+" "+"价格（元/小时）："+benjiquery.value(2).toString();
                 rec->ui->textEdit_price->append(rec_veiw);
             }
        }
        str=QString("SELECT *,(parking_others.`距离（千米）`*%1+parking_others.`价格（元/小时）`*%2) AS cost FROM parking_others ORDER BY cost").arg(list[1]).arg(list[2]);
        qDebug()<<str;
        benjiquery.exec(str);
        if(benjiquery.size()>0)
        {
             while (benjiquery.next()) {
                 rec_veiw=benjiquery.value(0).toString()+" "+"空闲车位："+benjiquery.value(4).toString()+" "+"距离（千米）："+benjiquery.value(1).toString()+" "+"价格（元/小时）："+benjiquery.value(2).toString()+" "+"预计花费："+benjiquery.value(2).toString();
                 rec->ui->textEdit_smart->append(rec_veiw);
             }
        }
        rec->show();

    }
    else if(list[0]=="反向寻车")
    {
         qDebug()<<"反向寻车";
         view_content=current_datetime+" 收到： ";
         ui->textEdit->append(view_content);
         view_content="请求寻找车辆 "+list[1];
         ui->textEdit->append(view_content);
         QString str=QString("SELECT 车位 from parking_real where 车牌='%1'").arg(list[1]);
         qDebug()<<str;
         QSqlDatabase benji=QSqlDatabase::database("benji");
         QSqlQuery benjiquery(benji);
         benjiquery.exec(str);
         if(benjiquery.size()>0)
         {
            benjiquery.first();
            QString av_site=benjiquery.value(0).toString();
            qDebug()<<av_site;
            send_content=list[0]+","+list[1]+","+av_site;
            view_content="车辆 "+list[1]+" 停在 "+av_site+" 车位";
            ui->textEdit->append(view_content);
            ui->textMoveWidget->setText(view_content);
            mSocket->write((send_content).toLocal8Bit());
         }
    }
    else if(list[0]=="缴费退场")
    {
         qDebug()<<"缴费退场";
         view_content=current_datetime+" 收到： ";
         ui->textEdit->append(view_content);
         view_content="车辆 "+list[1]+" 缴费离场";
         ui->textEdit->append(view_content);
         QString str=QString("SELECT parking_real.`车位` FROM parking_real WHERE parking_real.`车牌`='%1'").arg(list[1]);
         qDebug()<<str;
         QSqlDatabase benji=QSqlDatabase::database("benji");
         QSqlQuery benjiquery(benji);
         benjiquery.exec(str);
         if(benjiquery.size()>0)
         {
            benjiquery.first();
            QString av_site=benjiquery.value(0).toString();
            qDebug()<<av_site;
           // send_content=list[0]+","+list[1]+","+av_site;
            view_content="车辆 "+list[1]+" 离开 "+av_site+" 车位";
            ui->textEdit->append(view_content);
            ui->textMoveWidget->setText(view_content);
            //mSocket->write((send_content).toLocal8Bit());
            str=QString("update parking_real set 车牌='',状态='0',停车时间='' where 车位='%1'").arg(av_site);
            qDebug()<<str;
            qDebug()<<benjiquery.exec(str);
            Updatesql();
            UpdateParking();
            init_barchart();
            Update_barchart();
            Update_ringchart();
         }
    }
    else if(list[0]=="车辆就位")
    {
         qDebug()<<"车辆就位";
         view_content=current_datetime+" 收到： ";
         ui->textEdit->append(view_content);
         view_content="车辆 "+list[1]+" 已到达指定车位" +list[2];
         ui->textEdit->append(view_content);
        ui->textMoveWidget->setText(view_content);
        QString str=QString("update parking_real set 状态='1' where 车位='%1'").arg(list[2]);
        qDebug()<<str;
        QSqlDatabase benji=QSqlDatabase::database("benji");
        QSqlQuery benjiquery(benji);
        benjiquery.exec(str);
        Updatesql();
        UpdateParking();
        init_barchart();
        Update_barchart();
        Update_ringchart();

    }

}

void SmartHospital::client_dis()
{
    QTcpSocket *obj = (QTcpSocket*)sender();//掉线对象
    qDebug()<<obj->peerAddress().toString();//打印出掉线对象的ip
}

void SmartHospital::on_BG_Clicked(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
}
void SmartHospital::UpdateParking()
{
    qDebug()<<ParkingStaus;
    QStringList status;
    //qDebug()<<ParkingStaus<<ParkingStaus[0]<<ParkingStaus.size();
    ui->label_252->setText(QString::number(ParkingStaus.size()-ParkingStaus.count(1)));
    for(int i=0;i<ParkingStaus.size();++i)
    {
        // qDebug()<<ParkingStaus<<ParkingStaus[0];
        if(ParkingStaus[i]==0) status<<"font: 12pt '黑体';background-color: rgb(102, 255, 0);color: rgb(31, 31, 31); border-radius:2px;";
        if(ParkingStaus[i]==1) status<<"font: 12pt '黑体';background-color: rgb(255, 69, 23);color: rgb(31, 31, 31); border-radius:2px;";
        if(ParkingStaus[i]==2) status<<"font: 12pt '黑体';background-color:  rgb(255, 255, 0);color: rgb(31, 31, 31); border-radius:2px;";
        if(ParkingStaus[i]==3) status<<"font: 12pt '黑体';background-color: rgb(255, 162, 234);color: rgb(31, 31, 31); border-radius:2px;";
    }
    ui->label_001->setStyleSheet(status[0]);
    ui->label_002->setStyleSheet(status[1]);
    ui->label_003->setStyleSheet(status[2]);
    ui->label_004->setStyleSheet(status[3]);
    ui->label_005->setStyleSheet(status[4]);
    ui->label_006->setStyleSheet(status[5]);
    ui->label_007->setStyleSheet(status[6]);
    ui->label_008->setStyleSheet(status[7]);
    ui->label_009->setStyleSheet(status[8]);
    ui->label_010->setStyleSheet(status[9]);
    ui->label_011->setStyleSheet(status[10]);
    ui->label_012->setStyleSheet(status[11]);
    ui->label_013->setStyleSheet(status[12]);
    ui->label_014->setStyleSheet(status[13]);
    ui->label_015->setStyleSheet(status[14]);
    ui->label_016->setStyleSheet(status[15]);
    ui->label_017->setStyleSheet(status[16]);
    ui->label_018->setStyleSheet(status[17]);
    ui->label_019->setStyleSheet(status[18]);
    ui->label_020->setStyleSheet(status[19]);
    ui->label_021->setStyleSheet(status[20]);
    ui->label_022->setStyleSheet(status[21]);
    ui->label_023->setStyleSheet(status[22]);
    ui->label_024->setStyleSheet(status[23]);
    ui->label_025->setStyleSheet(status[24]);
    ui->label_026->setStyleSheet(status[25]);
    ui->label_027->setStyleSheet(status[26]);
    ui->label_028->setStyleSheet(status[27]);
    ui->label_029->setStyleSheet(status[28]);
    ui->label_030->setStyleSheet(status[29]);
    ui->label_031->setStyleSheet(status[30]);
    ui->label_032->setStyleSheet(status[31]);
    ui->label_033->setStyleSheet(status[32]);
    ui->label_034->setStyleSheet(status[33]);
    ui->label_035->setStyleSheet(status[34]);
    ui->label_036->setStyleSheet(status[35]);
    ui->label_037->setStyleSheet(status[36]);
    ui->label_038->setStyleSheet(status[37]);
    ui->label_039->setStyleSheet(status[38]);
    ui->label_040->setStyleSheet(status[39]);
    ui->label_041->setStyleSheet(status[40]);
    ui->label_042->setStyleSheet(status[41]);
    ui->label_043->setStyleSheet(status[42]);
    ui->label_044->setStyleSheet(status[43]);
    status.clear();
}

void SmartHospital::init_barchart()
{
    QStringList Parking_name;
    QBarSet* set0 = new QBarSet("最大容量");//声明QBarSet实例
    QBarSet* set1 = new QBarSet("空闲车位");
    
    QVector<int> Parking_maxnum,Parking_distance,Parking_num;
    QString str=QString("select * from parking_others ");
    qDebug()<<str;
    QSqlDatabase benji=QSqlDatabase::database("benji");
    QSqlQuery benjiquery(benji);
    benjiquery.exec(str);
    if(benjiquery.size()!=0)
    {
        while (benjiquery.next()) {
            Parking_name<<benjiquery.value(0).toString();
            Parking_distance<<benjiquery.value(1).toInt();
            Parking_maxnum<<benjiquery.value(3).toInt();
            * set0<<benjiquery.value(3).toInt();
            Parking_num<<benjiquery.value(3).toInt();
            *set1<<benjiquery.value(4).toInt();
        }
    }
    qDebug()<<Parking_name<<Parking_maxnum<<Parking_distance<<Parking_num;
    m_barChart = new QChart();
    m_barChart->setAnimationOptions(QChart::SeriesAnimations);//动画效果
    set0->setLabelColor(QColor(0,0,0));
    m_barSeries = new QBarSeries(m_barChart);
    m_barSeries->setLabelsVisible(true);
    m_barSeries->setLabelsPosition(QAbstractBarSeries::LabelsOutsideEnd);
    m_barSeries->append(set0);
    m_barSeries->append(set1);
    m_barChart->addSeries(m_barSeries);
    m_barChart->setTitle("周边各停车场空闲车位情况");
    
    //设置X轴参数
    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(Parking_name);//设置X周标签
    m_barChart->addAxis(axisX, Qt::AlignBottom); //将系列标签放到底下
    m_barSeries->attachAxis(axisX);
    
    //设置Y轴参数
    QValueAxis* axisY = new QValueAxis();
    axisY->setRange(0, 300);
    axisY->setLabelFormat("%u");
    m_barChart->addAxis(axisY, Qt::AlignLeft);//放到左边
    m_barSeries->attachAxis(axisY);
    
    
    //设置标签对应是可视的
    m_barChart->legend()->setVisible(true);
    m_barChart->legend()->setAlignment(Qt::AlignBottom);//放到底部
    
    ui->widget_bar->setChart(m_barChart);
    ui->widget_bar->setRubberBand(QChartView::RectangleRubberBand);//拉伸效果
    ui->widget_bar->setRenderHint(QPainter::Antialiasing);
    ui->widget_bar->chart()->setTheme(QChart::ChartThemeBlueCerulean);
}
void SmartHospital::Update_barchart()
{
    int A_0=0,A_1=0,A_2=0;
    int B_0=0,B_1=0,B_2=0;
    int C_0=0,C_1=0,C_2=0;
    for(int i=0;i<ParkingPartition.size();++i)
    {
        if(ParkingPartition[i]=="A")
        {
            if(ParkingStaus[i]==0) A_0++;
            if(ParkingStaus[i]==1) A_1++;
            if(ParkingStaus[i]==2) A_2++;
        }
        if(ParkingPartition[i]=="B")
        {
            if(ParkingStaus[i]==0) B_0++;
            if(ParkingStaus[i]==1) B_1++;
            if(ParkingStaus[i]==2) B_2++;
        }
        if(ParkingPartition[i]=="C")
        {
            if(ParkingStaus[i]==0) C_0++;
            if(ParkingStaus[i]==1) C_1++;
            if(ParkingStaus[i]==2) C_2++;
        }
    }
    qDebug()<<A_0<<A_1<<A_2<<B_0<<B_1<<B_2<<C_0<<C_1<<C_2;
    QBarSet* set0 = new QBarSet("临时车位");//声明QBarSet实例
    QBarSet* set1 = new QBarSet("占用车位");
    QBarSet* set2 = new QBarSet("固定车位");//声明QBarSet实例
    * set0<<A_0<<B_0<<C_0;
    * set1<<A_1<<B_1<<C_1;
    * set2<<A_2<<B_2<<C_2;
    m_barChart2 = new QChart();
    m_barChart2->setAnimationOptions(QChart::SeriesAnimations);//动画效果
    set0->setLabelColor(QColor(0,0,0));
    m_barSeries2 = new QBarSeries(m_barChart2);
    m_barSeries2->setLabelsVisible(true);
    m_barSeries2->setLabelsPosition(QAbstractBarSeries::LabelsOutsideEnd);
    m_barSeries2->append(set0);
    m_barSeries2->append(set1);
    m_barSeries2->append(set2);
    m_barChart2->addSeries(m_barSeries2);
    m_barChart2->setTitle("停车场各分区车位使用情况");

    //设置X轴参数
    QStringList Parking_part={"A区","B区","C区"};
    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(Parking_part);//设置X周标签
    m_barChart2->addAxis(axisX, Qt::AlignBottom); //将系列标签放到底下
    m_barSeries2->attachAxis(axisX);

    //设置Y轴参数
    QValueAxis* axisY = new QValueAxis();
    axisY->setRange(0, 15);
    axisY->setLabelFormat("%u");
    m_barChart2->addAxis(axisY, Qt::AlignLeft);//放到左边
    m_barSeries2->attachAxis(axisY);


    //设置标签对应是可视的
    m_barChart2->legend()->setVisible(true);
    m_barChart2->legend()->setAlignment(Qt::AlignBottom);//放到底部

    ui->widget_bar2->setChart(m_barChart2);
    ui->widget_bar2->setRubberBand(QChartView::RectangleRubberBand);//拉伸效果
    ui->widget_bar2->setRenderHint(QPainter::Antialiasing);
    ui->widget_bar2->chart()->setTheme(QChart::ChartThemeBlueCerulean);
}
void SmartHospital::Update_ringchart()
{
    int tem_0=ParkingStaus.count(0);
    int tem_1=ParkingStaus.count(1);
    int tem_2=ParkingStaus.count(2);
    int tem_3=ParkingStaus.count(3);
    int sum=ParkingStaus.size();
    qDebug()<<tem_0<<tem_1<<tem_2;
    qDebug()<<tem_0/sum*100;
    QString str1=QString::number(tem_0*100/sum);
    QString str2=QString::number(tem_1*100/sum);
    QString str3=QString::number(tem_2*100/sum);
    QString str4=QString::number(100-str1.toInt()-str2.toInt()-str3.toInt());
    QString ring_info=QString("临时车位,%1|已占有车位,%2|固定车位,%3|待就位车位,%4").arg(str1).arg(str2).arg(str3).arg(str4);
    qDebug()<<ring_info;
    ui->customRing->setOutPieInfos(ring_info);
}
void SmartHospital::init_linechart()
{
//    m_lineSeries= new QLineSeries();
//    m_lineSeries->append(0,2);
//    m_lineSeries->append(QPointF(2,6));
//    m_lineSeries->append(3,8);
//    m_lineSeries->append(7,9);
//    m_lineSeries->append(11,3);
//    *m_lineSeries << QPointF(11,2) << QPointF(15,5) << QPointF(18,4) << QPointF(19,2);
//    m_lineChart = new QChart();
//    // 将图例隐藏
//    m_lineChart->legend()->hide();
//    // 关联series，这一步很重要，必须要将series关联到QChart才能将数据渲染出来：
//    m_lineChart->addSeries(m_lineSeries);
//    // 开启OpenGL，QLineSeries支持GPU绘制，Qt其他有的图表类型是不支持的。
//    m_lineSeries->setUseOpenGL(true);
//    // 创建默认的坐标系（笛卡尔坐标）
//    //m_lineChart->createDefaultAxes();
//    //设置X轴参数
//    QValueAxis* axisX = new QValueAxis();
//    axisX->setRange(0, 24);
//    axisX->setLabelFormat("%u");
//    m_lineChart->addAxis(axisX, Qt::AlignBottom); //将系列标签放到底下
//    m_lineSeries->attachAxis(axisX);
    
//    //设置Y轴参数
//    QValueAxis* axisY = new QValueAxis();
//    axisY->setRange(0, 50);
//    axisY->setLabelFormat("%u");
//    m_lineChart->addAxis(axisY, Qt::AlignLeft);//放到左边
//    m_lineSeries->attachAxis(axisY);
//    // 设置图表标题
//    m_lineChart->setTitle(QStringLiteral("本日车位使用情况变化"));
//    ui->widget_line->setChart(m_lineChart);
//    ui->widget_line->setRubberBand(QChartView::RectangleRubberBand);//拉伸效果
//    ui->widget_line->setRenderHint(QPainter::Antialiasing);
//    ui->widget_line->chart()->setTheme(QChart::ChartThemeBlueCerulean);
}

void SmartHospital::on_ExitButton_clicked()
{
    qDebug()<<"退出";
    this->close();
}
