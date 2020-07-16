#include "recommend.h"
#include "ui_recommend.h"

recommend::recommend(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::recommend)
{
    ui->setupUi(this);
    setWindowTitle("智能推荐");
}

recommend::~recommend()
{
    delete ui;
}
