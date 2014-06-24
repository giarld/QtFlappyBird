#include "ground.h"

Ground::Ground(QWidget *parent)
    : QWidget(parent)
{
    this->setMaximumSize(380,60);
    this->setMinimumSize(380,60);
    sx=0;sy=0;

    timer=new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(groundM()));
    timer->start(8);
}

Ground::~Ground()
{

}

void Ground::groundM()
{
    sx=sx-1;
    if(sx<=-38)		//一个区间的滚动 在视觉上就是一直向左滚动
    {
        //timer->stop();
        sx=0;
    }
    update();
}

void Ground::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPixmap pixmap;
    pixmap.load(":/Images/ground.png");
    painter.drawPixmap(sx,sy,450,60,pixmap);
}

void Ground::play()
{
    timer->start(8);
}

void Ground::stop()
{
    timer->stop();
}
