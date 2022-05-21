#ifndef MYSLIDER_H
#define MYSLIDER_H
#include<QSlider>
#include<QMouseEvent>
#include<QDebug>
#include<QPainter>
#include<QLabel>
#include<QPixmap>
class myslider : public QSlider
{
    Q_OBJECT
   public:
        myslider(QWidget *parent = 0) : QSlider(parent)
        {
        }
   protected:
       void mousePressEvent(QMouseEvent *ev);//重写QSlider的mousePressEvent事件
       void mouseMoveEvent(QMouseEvent* e);

   signals:
       void costomSliderClicked();//自定义的鼠标单击信号，用于捕获并处理
       void painte() ;

};

#endif // MYSLIDER_H
