
#include "displayModel.h"
#include <stdio.h>

//#ifndef QT_NO_CAST_FROM_ASCII
//#define QT_NO_CAST_FROM_ASCII
//#endif


PatternView::PatternView(int w, int h, QWidget *parent)
    :QWidget(parent)
{
    resize(w, h);
    //printf("MarkView size: (%d, %d)\n", width(), height());
    resizeImg=cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);
    rgbImg=cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 3);
    assert(resizeImg && rgbImg);
   // widthRatio=1;
   // heighRatio=1;
}

void PatternView::receive_image(const IplImage *img){
    assert(img);
    assert(img->nChannels==1 && img->depth==IPL_DEPTH_8U);

    if(img->width==width() && img->height==height())
        cvCopy(img, resizeImg);
    else
        cvResize(img, resizeImg);
    //widthRatio=1.0*width()/img->width;
   // heighRatio=1.0*height()/img->height;
   // printf("w=%d, %d, ratio=%f, %f\n", width(), height(),widthRatio, heighRatio);
    cvCvtColor(resizeImg, rgbImg, CV_GRAY2RGB);
}

void PatternView::paintEvent(QPaintEvent*){
    QPainter painter(this);
    QImage qImage((const uchar*)rgbImg->imageData,width(),
                  height(),rgbImg->widthStep,QImage::Format_RGB888);
    painter.drawImage(QPoint(0,0),qImage);
}



FocusAidView::FocusAidView(int w, int h, int zoomRaito, QWidget *parent)
    : QWidget(parent), zoomTimes(zoomRaito), leftTop(cvPoint(0,0))
{
    resize(w, h);
    focusW = width()/zoomTimes;
    focusH = height()/zoomTimes;
    if(width()%zoomTimes!=0)
        focusW++;
    if(height()%zoomTimes!=0)
        focusH++;
    resizeImg=cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);
    rgbImg=cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 3);
    assert(resizeImg && rgbImg);
}

void FocusAidView::set_zoom_times(int times) {
    zoomTimes= MAX(times, 1);
    focusW = width()/zoomTimes;
    focusH = height()/zoomTimes;
    if(width()%zoomTimes!=0)
        focusW++;
    if(height()%zoomTimes!=0)
        focusH++;
}

void FocusAidView::set_left_top(CvPoint lefttop){
    leftTop=lefttop;
}

CvRect FocusAidView::get_focus_area()const{
    return cvRect(leftTop.x, leftTop.y, focusW, focusH);
}

void FocusAidView::receive_image(const IplImage * img){

    leftTop.x=MAX(leftTop.x, 0);
    leftTop.y=MAX(leftTop.y, 0);
    leftTop.x=MIN(leftTop.x, img->width-focusW);
    leftTop.y=MIN(leftTop.y, img->height-focusH);
    int row, col, rts, cts;
    char *focusImgPtr= resizeImg->imageData;
    char* srcPtr= img->imageData+leftTop.y*img->widthStep+leftTop.x;
    char* srcRowPtr=srcPtr;
    int srcWidth=img->widthStep;
    for(row=0, rts=0; row<height(); row++){
        for(col=0,cts=0; col<width(); col++){
            *(focusImgPtr++) = *srcPtr;
            cts++;
            if(cts==zoomTimes){
                cts=0;
                srcPtr++;
            }
        }
        rts++;
        if(rts==zoomTimes){
            rts=0;
            srcRowPtr+=srcWidth;
        }
        srcPtr=srcRowPtr;
    }
    cvCvtColor(resizeImg, rgbImg, CV_GRAY2RGB);
}

void FocusAidView::paintEvent(QPaintEvent*){
    QPainter painter(this);
    QImage qImage((const uchar*)rgbImg->imageData,width(),
                  height(),rgbImg->widthStep,QImage::Format_RGB888);
    painter.drawImage(QPoint(0,0),qImage);
}

