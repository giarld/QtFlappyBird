#ifndef GROUND_H
#define GROUND_H

#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QTimer>

class Ground : public QWidget
{
    Q_OBJECT

public:
    Ground(QWidget *parent = 0);
    ~Ground();
    void stop();
    void play();

protected:
    void paintEvent(QPaintEvent *);

private slots:
    void groundM();

private:
    QTimer *timer;
    int sx,sy;
};

#endif // GROUND_H
