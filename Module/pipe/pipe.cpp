#include "pipe.h"

Pipe::Pipe(QWidget *parent)
    : QWidget(parent)
{
    Gap=140;
    setGapSize(Gap);
}

Pipe::~Pipe()
{

}


void Pipe::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing,true);
    QPixmap pix;
    pix.load(":/Images/pipe1.png");
    painter.drawPixmap(0,0,70,250,pix);
    pix.load(":/Images/pipe2.png");
    painter.drawPixmap(0,250+Gap,70,300,pix);
}

int Pipe::getH1()
{
    return 250;
}

int Pipe::getH2()
{
    return 300;
}

int Pipe::getGap()
{
    return Gap;
}


void Pipe::setGapSize(int w)
{
    this->setFixedSize(70,w+600);
    this->Gap=w;
    update();
}
