#ifndef DISPLAYMODEL_H
#define DISPLAYMODEL_H

#include <QtGui/QGraphicsView>
#include <QMouseEvent>
#include "cv.h"

static const int pointSize=6;
static const int rotatePointR=4;
static const int sensitiveRange=10;


class PatternView: public QWidget
{
public:
    PatternView(int w,int h, QWidget *parent = 0);
    void receive_image(const IplImage*img);
protected:
    void paintEvent(QPaintEvent *event);
private:
    IplImage* resizeImg;
    IplImage* rgbImg;
};

class FocusAidView: public QWidget
{
public:
    FocusAidView(int w,int h, int zoomRaito, QWidget *parent = 0);
    void receive_image(const IplImage * img);
    void set_zoom_times(int times);
    void set_left_top(CvPoint lefttop);
    CvRect get_focus_area()const;
    int get_zoom_times()const{return zoomTimes;};

protected:
    void paintEvent(QPaintEvent *event);

private:
    int zoomTimes;
    CvPoint leftTop;
    int focusW, focusH;
    IplImage* resizeImg;
    IplImage* rgbImg;
};


#endif // DISPLAYMODEL_H
