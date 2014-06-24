#include "mainwindow.h"
#include <QDesktopWidget>
#include <QMessageBox>
#include <ctime>
#include <iostream>
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    //构造函数创建了整个游戏界面和时间处理规则。对象的创建顺序就是最终的窗口层次顺序
    // this->setMaximumSize(380,500);
    //  this->setMinimumSize(380,500);

    this->setFixedSize(380,500);	//锁定窗口的大小为380x500  效果等于上面两句
    this->setWindowIcon(QIcon(":/Images/bird1.png"));
    this->move((QApplication::desktop()->width()-this->width())/2,(QApplication::desktop()->height()-this->height())/2);	///这样做可以让窗口居中于屏幕显示，需要引入 QDesktopWidget

    for(int i=0;i<pipeCount;i++)	///创建管道对象
        pipe[i]=new Pipe(this);

    createPipe();
    pipeTimer =new QTimer(this);	//创建管道的Timer 以下的三个connect说明了其处理的动作不只一个
    connect(pipeTimer,SIGNAL(timeout()),this,SLOT(pipeAction()));
    connect(pipeTimer,SIGNAL(timeout()),this,SLOT(collisDete()));
    connect(pipeTimer,SIGNAL(timeout()),this,SLOT(scoreDete()));
    pipeTValue=8;

    birds=new Bird(this);
    birds->move(60,250);

    this->fpV[0]=-3;
    this->fpV[1]=-4;
    this->fpV[2]=-3;
    this->fpV[3]=-2;
    this->fpV[4]=-1;
    this->fpV[5]=-1;
    this->fpV[6]=-1;
    this->fpV[7]=0;
    this->fpV[8]=1;
    this->fpV[9]=1;
    this->fpV[10]=2;
    this->fpV[11]=2;
    this->fpV[12]=2;
    this->fpV[13]=3;
    this->fpV[14]=3;

    this->fpp=0;


    birdTimer=new QTimer(this);
    connect(birdTimer,SIGNAL(timeout()),this,SLOT(birdAction()));
    //birdTimer->start(20);
    birdV=0;
    gamemod=redy;		//游戏状态出事化为redy

    score=0;
    top=0;
    loadTop();		//载入得分记录

    scoreLCD=new FBNumLCD(this);	///计分板的实现
    scoreLCD->setShowHead(false);	///不显示多余位数
    scoreLCD->setValue(0);
    //scoreLCD->setFixedSize(28*4,36);
    scoreLCD->move(140,-100);

    scoreBoard=new ScoreBoard(this);
    scoreBoard->move(-350,100);
    scobTimer=new QTimer(this);
    connect(scobTimer,SIGNAL(timeout()),this,SLOT(scbAction()));


    thisGround=new Ground(this);
    thisGround->move(0,450);

    myredyBoard=new RedyBoard(this);
    myredyBoard->setFixedSize(380,500);
    myredyBoard->move(0,0);

    replay=new QLabel(this);		///这就是那个万恶的按钮 他的实现完全靠一个区域限制
    QPixmap pix;
    pix.load(":/Images/replay.png");		///所有的素材被写在资源文件中 编译时会被加到应用程序中去.调用资源文件只需要使用"："即可
    replay->setPixmap(pix);
    replay->setFixedSize(140,80);
    replay->move(120,-400);

    //////sound
    playList=new QMediaPlaylist;
    QFileInfo info;
    info.setFile("sounds/sfx_wing.mp3");
    playList->addMedia(QUrl::fromLocalFile(info.absoluteFilePath()));
    info.setFile("sounds/sfx_point.mp3");
    playList->addMedia(QUrl::fromLocalFile(info.absoluteFilePath()));
    info.setFile("sounds/sfx_die.mp3");
    playList->addMedia(QUrl::fromLocalFile(info.absoluteFilePath()));
    info.setFile("sounds/sfx_hit.mp3");
    playList->addMedia(QUrl::fromLocalFile(info.absoluteFilePath()));
    info.setFile("sounds/sfx_swooshing.mp3");
    playList->addMedia(QUrl::fromLocalFile(info.absoluteFilePath()));

    playList->setPlaybackMode(QMediaPlaylist::CurrentItemOnce);

    playList_wing=new QMediaPlaylist;
    info.setFile("sounds/sfx_wing.mp3");
    playList_wing->addMedia(QUrl::fromLocalFile(info.absoluteFilePath()));
    playList_wing->setPlaybackMode(QMediaPlaylist::CurrentItemOnce);
    playList_wing->setCurrentIndex(1);

    media=new QMediaPlayer;
    media->setVolume(100);
    media->setPlaylist(playList_wing);

    isFlag=-1;
    media2=new QMediaPlayer;
    media2->setPlaylist(playList);
    media2->setVolume(100);
    isHit=0;

    this->setWindowTitle("Qt FlappyBird");		///设置标题
}

void MainWindow::createPipe()		//初始化管道。使其以一定次序排在地图之外
{
    int startx=500+380;		//第一个管道的位置
    pipeXgap=200;		///间距
    int pipR;			///会产生一个垂直位置的随机数
    qsrand(time(NULL));		///qrand是qt的随机数函数 用法个c的rand一样  也需要初始化种子

    for(int i=0;i<pipeCount;i++)
    {
        pipR=qrand()%200;

        pipe[i]->move(startx+i*pipeXgap,-200+pipR);
        lastPipe=i;		//很重要 设置最后一根管道的编号 为后面的管道循环奠定基础
    }
}

MainWindow::~MainWindow()
{

}

void MainWindow::paintEvent(QPaintEvent *)		//重载的绘图事件。用来产生背景
{
    QPainter painter(this);
    QPixmap pixmap;
    pixmap.load(":/Images/bg.png");
    painter.drawPixmap(0,0,380,450,pixmap);
}

void MainWindow::mousePressEvent(QMouseEvent *event)	//鼠标事件
{
    //左键或右键都能控制鸟的运动 并且为防止飞出地图 曾加了birds->pos().y()>0
    if((event->button()==Qt::LeftButton||event->button()==Qt::RightButton)&&birds->pos().y()>0)
    {
        if(gamemod==stop)		//如果游戏是停止 也就是失败了的状态下，吐过计分板到位才能触发事件
        {
            if(isScobOk)
                if((event->pos().x()>=120&&event->pos().x()<=260)&&(event->pos().y()>=400&&event->pos().y()<=480))
                {
                    ///这里 当点击开始按钮 计分板会退出地图，鸟会归位，管道会归位，游戏状态改为redy，计分板会清空，路面会开始运动
                    gameRedy();
                    playSound(s_sw);
                }

        }
        else
        {
            mainAction();  //剩下的事件处理 因为是键盘鼠标通用 所以写在函数中
        }

    }
}



void MainWindow::keyPressEvent(QKeyEvent *event)
{
    //键盘处理 空格 上键的可操控
    if((event->key()==Qt::Key_Space||event->key()==Qt::Key_Up)&&birds->pos().y()>0)
    {
        mainAction();
    }
}

void MainWindow::mainAction()
{
    //通用事件处理
    if(gamemod==redy)	//如果是redy：开始游戏，鸟开始运动，管道开始运动
    {
        gameStart();
        timedata=8;
        birdTimer->start(timedata);
        pipeTimer->start(pipeTValue);
        fpp=0.0;
        birdV=fpV[int(fpp)];		///每次触发 鸟都会向上飞 也就是速度是负数。
        emit birds->fly();	///这个信号是Bird类的 用于让鸟抬头
    }
    else if(gamemod==start)	//如果游戏是开始状态，只处理鸟的飞行姿态
    {
        fpp=0.0;
        birdV=fpV[int(fpp)];
        timedata=8;
        birdTimer->setInterval(timedata);
        emit birds->fly();
    }
    playWingSound();
}

void MainWindow::birdAction()
{
    //bird的运动时间。这里是决定鸟运动数度 操作难度的地方。
    birds->move(birds->pos().x(),birds->pos().y()+birdV);

    if(fpp<14.0)
        fpp+=0.2;
    else
        fpp=14.0;
    birdV=fpV[int(fpp)];

    if(birds->pos().y()+birds->height()>=450)
    {
        birds->move(birds->pos().x(),450-birds->height()+5);
        birdTimer->stop();
        isHit=true;
        playSound(s_hit);
        gameLose();
        birds->setRale(90);
    }

}

void MainWindow::pipeAction()
{
    //管道的动画，重点是管道离开地图后将重新回到右侧并接替lastPipe的地位，产生一个新的高度
    int pipR;
    for(int i=0;i<pipeCount;i++)
    {
        pipe[i]->move(pipe[i]->pos().x()-1,pipe[i]->pos().y());
        if(pipe[i]->pos().x()<-100)
        {
            pipR=qrand()%200;
            pipe[i]->move(pipe[lastPipe]->pos().x()+pipeXgap,-200+pipR);
            lastPipe=i;
        }
    }
}

void MainWindow::collisDete()
{
    ///碰撞检测，判定是否优秀看这里。不注释
    int birdRx=birds->pos().x()+30;
    int birdDy=birds->pos().y()+30;
    for(int i=0;i<pipeCount;i++)
    {
        if(birdRx>=pipe[i]->x()&&birds->pos().x()<=pipe[i]->pos().x()+pipe[i]->width()-10)
        {
            if(birds->y() <= (pipe[i]->y()+pipe[i]->getH1()) || birdDy >= (pipe[i]->y()+pipe[i]->getH1()+pipe[i]->getGap()))
                gameLose();
        }
    }
}

void MainWindow::scoreDete()
{
    //分数检测，实质也是一个碰撞检测
    for(int i=0;i<pipeCount;i++)
    {
        if(birds->pos().x()+birds->width()>=pipe[i]->pos().x()+35&&birds->pos().x()+birds->width()<=pipe[i]->pos().x()+40&&cx)
        {
            playSound(s_point);
            this->score+=1;
            scoreLCD->setValue(score);
            //birds->fly();
            if(score>=1000)
            {
                scoreLCD->move(90+14*3,50);
            }
            else if(score>=100)
            {
                scoreLCD->move(90+14*2,50);
            }
            else if(score>=10)
            {
                scoreLCD->move(90+14,50);
            }
            cx=0;
        }

        if(birds->pos().x()>=pipe[i]->pos().x()+68&&birds->pos().x()<=pipe[i]->pos().x()+73)
            cx=1;
    }
}


void MainWindow::scbAction()
{
    ///失败时的记分牌动画
    scoreBoard->move(scoreBoard->pos().x()+1,scoreBoard->pos().y());
    if(scoreBoard->pos().x()>=40)
    {
        scoreBoard->move(40,scoreBoard->pos().y());
        scobTimer->stop();
        replay->move(120,400);
        isScobOk=1;
    }
}

void MainWindow::gameRedy()
{
    myredyBoard->show();
    scoreBoard->move(-350,100);
    birds->move(60,250);
    replay->move(120,-400);
    createPipe();
    birds->setRale(-50);
    gamemod=redy;
    scoreLCD->setValue(0);
    thisGround->play();
    birds->play();
}

void MainWindow::gameLose()
{
    //游戏失败时的处理：记分牌滑出，游戏状态失败，地面停止，鼠标键盘时间锁定，计算得分
    isScobOk=0;
    gamemod=stop;
    birds->stop();
    pipeTimer->stop();
    thisGround->stop();
    if(!isHit)
    {
        playSound(s_die);
    }
    else
        isHit=false;

    if(score>top)	///如果创造了新纪录，保存记录
    {
        top=score;
        saveTop();
    }
    scoreBoard->setScore(score,top);
    scobTimer->start(3);
    scoreLCD->move(140,-100);

}

void MainWindow::gameStart()
{
    //开始游戏的动作
    gamemod=start;
    myredyBoard->close();
    cx=1;
    score=0;
    scoreLCD->move(90,50);

}

void MainWindow::saveTop()
{
    //保存记录，二进制保存，存储在top.d文件下。
    QFile file("top.d");
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    out<<this->top;
}

void MainWindow::loadTop()
{
    //读取记录 在构造函数里触发
    QFile file("top.d");
    if(file.open(QIODevice::ReadOnly))
    {
        QDataStream in(&file);
        in>>this->top;
    }
    else
        top=0;
}

void MainWindow::playWingSound()
{
    media->stop();

    media->play();
}

void MainWindow::playSound(int flag)
{
    if(isFlag!=flag)
    {
        playList->setCurrentIndex(flag);
    }
    media2->play();
}
