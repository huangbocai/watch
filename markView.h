#ifndef MARKVIEW_H
#define MARKVIEW_H

#include <QtGui/QGraphicsView>
#include <QMouseEvent>
#include <list>
#include "imageMeasure.hh"
#include "cv.h"

class RectangleFrame{
public:
    typedef enum{lt=0, top, rt, right, rb,  bottom,  lb, left, MOVE, nochange}ChangeType;
    RectangleFrame(const Point& leftTop=Point(), double width=0, double height=0, double deg=0);
    void area_change(ChangeType type, double x, double y);
    void move(double x, double y);
    void rotate(double deg);
    void zoom(float widthRatio, float heightRatio);
    void set_resizable(bool yes){resizable=yes;}
    void set_rotatable(bool yes) {rotatable=yes;}
    void set_visible(bool yes){visible=yes;}
    const Point& get_conner(int index)const;
    Point get_center()const;
    bool get_visible()const {return visible;}
    bool get_resizable() const{return resizable;}
    bool get_rotatable() const{return rotatable;}
    const Point& get_top_left()const {return conner[0];}
    const Point& get_bottom_right()const {return conner[2];}
    float get_width() const ;
    float get_height() const;
    ChangeType get_ready_change_type(double x, double y, double range)const;
    //double get_deg();
private:
    void left_move(double x, double y);
    void right_move(double x, double y);
    void top_move(double x, double y);
    void bottom_move(double x, double y);
    void left_top_move(double x, double y);
    void right_top_move(double x, double y);
    void left_bottom_move(double x, double y);
    void right_bottom_move(double x, double y);

    void conner_move(double x, double y, Vector2& vw, Vector2& vh);
    Point conner[4];
    bool visible, resizable, rotatable;
};



class MarkView: public QWidget
{
    Q_OBJECT
public:
    typedef enum{single_ccd=0,using_ccd1, using_ccd2, without_ccd, comincate_fail} CCDStauts;
    MarkView(QWidget *parent = 0);
    void receive_image(const IplImage*img);
    void set_default_pattern_frame();
    RectangleFrame get_pattern_frame();
    void set_default_search_frame();
    RectangleFrame get_search_frame();

    void set_search_frame(CvRect rect);
    void set_diamond_pos(const list<Point>& pos);
    void set_diamond_sum(int sum);
    void set_circle(bool visible, double centerx, double centery, double R);
    void set_focus_box(bool visible, CvRect rect= cvRect(0, 0, 0, 0));
    void set_ccd_status(CCDStauts status){ccdStatus=status;}
    void set_pass(bool val);
    //const QImage& get_view_image(int width, int height);

    ~MarkView();
public slots:
    void cancel_area_select();

signals:
    void left_press(int x, int y);

protected:
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent ( QMouseEvent * event);
    void mouseReleaseEvent( QMouseEvent * event);
    //RectangleFrame::ChangeType frameChangeType;

private:
    static const int sensitiveRange=10;
    static const int pointSize=6;
    void draw_cross_line(QImage& qImage);
    void draw_diamond_pos(QImage& qImage);
    void draw_frame(QImage& qImage, const RectangleFrame& frame, const QPen& pen);
    void draw_status_text(QImage& qImage);
    void draw_ccd_status(QImage& qImage);
    void draw_key_not_passed(QImage& qImage);
    void draw_circle();
    void draw_focus_box();

    IplImage* resizeImg;
    IplImage* dispImg;
    QImage paintQImage;
    double widthRatio;
    double heighRatio;

    QImage* outQImage;
    IplImage* outGrayImage;
    IplImage* outIplImage;
    double outWidthRatio;
    double outHeighRatio;


    RectangleFrame patternFrame;
    RectangleFrame searchFrame;

    //RectangleFrame similarFrame[4];
    //RectangleFrame searchFrame[4];

    list<Point> diamondPos;
    int diamondSum;

    enum{IDEL, READY, CHANGING}state;
    int cursorx, cursory;

    bool showCircle;
    double cx, cy, r;

    bool showFocusBox;
    CvRect focusBox;

    CCDStauts ccdStatus;

    bool keyPass;
};

#endif // MARKVIEW_H
