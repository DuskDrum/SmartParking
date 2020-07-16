#ifndef RECOMMEND_H
#define RECOMMEND_H

#include <QWidget>

namespace Ui {
class recommend;
}

class recommend : public QWidget
{
    Q_OBJECT

public:
    explicit recommend(QWidget *parent = nullptr);
    ~recommend();
    Ui::recommend *ui;
private:

};

#endif // RECOMMEND_H
