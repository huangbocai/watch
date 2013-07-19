#include "markView.h"
#include <stdio.h>

MarkView::MarkView( QWidget *parent)
    :QWidget(parent)
{
    resize(parent->width(), parent->height());
//    printf("MarkView size: (%d, %d)\n", width(), height());
    resizeImg = cvCreateImage(cvSize(width(), height()),IPL_DEPTH_8U, 1);
    dispImg=cvCreateImageHeader(cvSize(width(), height()), IPL_DEPTH_8U, 3);
    assert(dispImg);
    paintQImage=QImage(width(), height(), QImage::Format_RGB888);
    dispImg->imageData=(char*)paintQImage.bits();
    widthRatio=1;
    heighRatio=1;

    outQImage=NULL;
    outGrayImage=NULL;
    outIplImage=NULL;
    outWidthRatio=0;
    outHeighRatio=0;

    setMouseTracking(true);
    state=IDEL;
    showFocusBox=false;
    keyPass=false;
    diamondSum=0;
    ccdStatus=single_ccd;
}

MarkView::~MarkView(){
    cvReleaseImage(&resizeImg);
    cvReleaseImageHeader(&dispImg);
    if(outIplImage)
        cvReleaseImageHeader(&outIplImage);
    if(outQImage)
        delete outQImage;
    if(outGrayImage)
        cvReleaseImage(&outGrayImage);
}

void MarkView::receive_image(const IplImage* img){
    assert(img);
    assert(img->nChannels==1 && img->depth==IPL_DEPTH_8U);

    if(resizeImg->width!=img->width || resizeImg->height!=img->height)
        cvResize(img, resizeImg);
    else
        cvCopy(img, resizeImg);

    widthRatio=1.0*width()/img->width;
    heighRatio=1.0*height()/img->height;
//    printf("w=%d, %d, ratio=%f, %f\n", width(), height(),widthRatio, heighRatio);
}


void MarkView::set_default_pattern_frame(){
    patternFrame=RectangleFrame(Point(width()/3, height()/3), width()/3, height()/3);
    patternFrame.set_visible(true);
    patternFrame.set_resizable(true);
    searchFrame.set_visible(false);
    state=READY;
}

RectangleFrame MarkView::get_pattern_frame(){
    patternFrame.set_visible(false);
    patternFrame.zoom(1/widthRatio, 1/heighRatio);
    state=IDEL;
    return patternFrame;
}

void MarkView::set_default_search_frame(){
    Point tl=searchFrame.get_top_left();
    Point br=searchFrame.get_bottom_right();
    if(tl.x()<10 || tl.y()<10 ||br.x()>width()-10, br.y()>height()-10
            ||searchFrame.get_width()<60 || searchFrame.get_height()<60)
        searchFrame=RectangleFrame(Point(10,10),width()-20, height()-20);
    searchFrame.set_visible(true);
    searchFrame.set_resizable(true);
    patternFrame.set_visible(false);
    state=READY;
}

RectangleFrame MarkView::get_search_frame(){
    searchFrame.set_resizable(false);
    RectangleFrame tmp=searchFrame;
    tmp.zoom(1/widthRatio, 1/heighRatio);
    //printf("searchFrame:%f,%f\ntemp:%f,%f\n,ratio:%f,%f\n\n",searchFrame.get_top_left().x(),
    //       searchFrame.get_top_left().y(),tmp.get_top_left().x(),tmp.get_top_left().y(),widthRatio,heighRatio);
    state=IDEL;
    return tmp;
}

void MarkView::set_search_frame(CvRect rect){
    searchFrame=RectangleFrame(Point(rect.x, rect.y), rect.width, rect.height);
    searchFrame.zoom(widthRatio, heighRatio);
    searchFrame.set_visible(true);
}

void MarkView::cancel_area_select(){
    if(state==READY){
       // frameChangeType=RectangleFrame::nochange;
        patternFrame.set_visible(false);
        searchFrame.set_visible(false);
        state=IDEL;
    }
}

void MarkView::mouseMoveEvent(QMouseEvent *event){
    static  RectangleFrame::ChangeType oldChangeType=RectangleFrame::nochange;

    QPoint pos= event->pos();
    int x=pos.x();
    int y=pos.y();
    if(x<0 ||y<0 ||x>=width() ||y>=height()){
        event->accept();
        return;
    }
    if(state==READY){
        RectangleFrame::ChangeType changeType;
        if(patternFrame.get_visible())
            changeType=patternFrame.get_ready_change_type(x,y,sensitiveRange);
        else
            changeType=searchFrame.get_ready_change_type(x,y,sensitiveRange);
        //change the cursor shape
        if(changeType!=oldChangeType){
            oldChangeType=changeType;
            switch(changeType){
            case RectangleFrame::MOVE:
                setCursor(Qt::SizeAllCursor);
                break;
            case RectangleFrame::top:
            case RectangleFrame::bottom:
                setCursor(Qt::SizeVerCursor);
                break;
            case RectangleFrame::left:
            case RectangleFrame::right:
                setCursor(Qt::SizeHorCursor);
                break;
            case RectangleFrame::lt:
            case RectangleFrame::rb:
                setCursor(Qt::SizeFDiagCursor);
                break;
            case RectangleFrame::rt:
            case RectangleFrame::lb:
                setCursor(Qt::SizeBDiagCursor);
                break;
            default:
                setCursor(Qt::ArrowCursor);
            }
        }
    }
    else if(state==CHANGING){
        RectangleFrame temp;
        if(patternFrame.get_visible()){
            temp=patternFrame;
            temp.area_change(oldChangeType, x-cursorx,y-cursory);
            if(temp.get_top_left().x()>0 && temp.get_top_left().y()>0
                    && temp.get_bottom_right().x()<width()-1
                    && temp.get_bottom_right().y()<height()-1)
                patternFrame=temp;
        }
        else if(searchFrame.get_visible()){
            temp=searchFrame;
            temp.area_change(oldChangeType, x-cursorx,y-cursory);
            if(temp.get_top_left().x()>0 && temp.get_top_left().y()>0
                    && temp.get_bottom_right().x()<width()-1
                    && temp.get_bottom_right().y()<height()-1)
                searchFrame=temp;
        }
        cursorx=x;
        cursory=y;
    }


    event->accept();
}

void MarkView::mousePressEvent(QMouseEvent *event){
    if(event->button()==Qt::LeftButton){
        if(state==READY){
            cursorx=event->pos().x();
            cursory=event->pos().y();
            state=CHANGING;
        }
        emit left_press(event->pos().x()/widthRatio, event->pos().y()/heighRatio);
    }
    event->accept();
}

void MarkView::mouseReleaseEvent(QMouseEvent *event){
    if(event->button()==Qt::LeftButton){
        if(state==CHANGING)
            state=READY;
    }
}

void MarkView::set_diamond_pos(const list<Point> &pos){
    diamondPos=pos;
}

void MarkView::set_diamond_sum(int sum){
    diamondSum=sum;
}

void MarkView::set_circle(bool visible, double centerx, double centery, double R){
    showCircle=visible;
    cx=centerx*widthRatio;
    cy=centery*heighRatio;
    r=R*(widthRatio+heighRatio)/2;
}

void MarkView::set_focus_box(bool visible, CvRect rect){
    showFocusBox=visible;
    if(visible){
        focusBox.x=rect.x*widthRatio;
        focusBox.y=rect.y*heighRatio;
        focusBox.width= rect.width*widthRatio;
        focusBox.height=rect.height*heighRatio;
        //printf("set focus box, (%d, %d), (%d, %d) \n", focusBox.x, focusBox.y, focusBox.width, focusBox.height );
    }
}

void MarkView::set_pass(bool val){
    keyPass=val;
}

void MarkView::draw_cross_line(QImage& qImage){
    QPainter painter(&qImage);
    QPen pen(QColor(0,0,255));
    pen.setStyle(Qt::DashLine);
    painter.setPen(pen);
    painter.drawLine(0, qImage.height()/2, qImage.width(), qImage.height()/2);
    painter.drawLine(qImage.width()/2, 0, qImage.width()/2, qImage.height());
}

void MarkView::draw_diamond_pos(QImage &qImage){
    QPainter painter(&qImage);
    QPen pen(QColor(255,0,0));
    painter.setPen(pen);
    list<Point>::iterator it;
    int x, y;
    float wr=widthRatio;
    float hr=heighRatio;
    for(it=diamondPos.begin(); it!=diamondPos.end(); it++){
        x=it->x()*wr+0.5;
        y=it->y()*hr+0.5;
        painter.drawLine(x-5, y, x+5, y);
        painter.drawLine(x, y-5, x, y+5);
    }
}

void MarkView::draw_frame(QImage& qImage, const RectangleFrame& frame, const QPen& pen){
    int i;
    bool inWin;
    if(!frame.get_visible())
        return;
    QPainter painter(&qImage);
    painter.setPen(pen);
    bool resizable=frame.get_resizable();
    for(i=0;i<4;i++){
        const Point& p1=frame.get_conner(i);
        const Point& p2=frame.get_conner((i+1)%4);
        inWin=false;
        if(p1.x()>=0 && p1.x()<width() && p1.y()>=0 && p1.y()<height()){
            inWin=true;
            if(resizable)
                painter.fillRect(p1.x()-pointSize/2+0.5, p1.y()-pointSize/2+0.5, pointSize, pointSize,pen.color());
        }
        if(p2.x()>=0 && p2.x()<width() && p2.y()>=0 && p2.y()<height())
            inWin=true;
        if(inWin){
            painter.drawLine(p1.x()+0.5, p1.y()+0.5, p2.x()+0.5, p2.y()+0.5);
            if(resizable)
                painter.fillRect((p1.x()+p2.x())/2-pointSize/2+0.5,
                                 (p1.y()+p2.y())/2-pointSize/2+0.5, pointSize, pointSize,pen.color());
        }
    }
}
/*
void MarkView::draw_frame(QImage &qImage, const CircleFrame &frame, const QPen &pen)
{
    QPainter painter(&qImage);
    painter.setPen(pen);
    bool resizable = true;
    bool inWin = true;
    for(int i=0;i<4;i++){
        const Point& p1=frame.get_conner(i);
        if(!(p1.x()>=0 && p1.x()<width() && p1.y()>=0 && p1.y()<height())){
            inWin=false;
        }
    }
    if(inWin){
        const Point& center = frame.get_center();
        double radius = frame.get_radius();
        painter.drawEllipse(QPointF(center.x(),center.y()),radius,radius);
        if(resizable){
            for(int i=0; i<4; i++){
                const Point& p=frame.get_conner(i);
                painter.fillRect(p.x()-pointSize/2+0.5,
                                 p.y()-pointSize/2+0.5, pointSize, pointSize,pen.color());
            }
        }
    }
}
*/

void MarkView::draw_status_text(QImage &qImage){
    char buf[128];
    sprintf(buf,"图中钻石数：%d \t可用钻石总数:%d", (int)diamondPos.size(),diamondSum);
    QPainter painter(&qImage);
    painter.setPen(QColor(0,255,0));
    painter.drawText(5, 25, QString::fromUtf8(buf));
}

void MarkView::draw_ccd_status(QImage& qImage){
    char buf[64];

    switch(ccdStatus){
    case single_ccd:
        return;
    case using_ccd1:
        strcpy(buf, "CCD1");
        break;
    case using_ccd2:
        strcpy(buf, "CCD2");
        break;
    case without_ccd:
        strcpy(buf, "未连接CCD");
        break;
    case comincate_fail:
        strcpy(buf,"CCD通讯出错");
        break;
    default:
        return;
    }
    QPainter painter(&qImage);
    painter.setFont(QFont("Sans",10,75));
    painter.setPen(QColor(255,0,0));
    painter.drawText(5, qImage.height()-5, QString::fromUtf8(buf));
}

void MarkView::draw_key_not_passed(QImage &qImage){
    if(!keyPass){
        QPainter painter(&qImage);
        painter.setFont(QFont("Sans",15,75));
        painter.setPen(QColor(255,0,0));
        painter.drawText(qImage.width()/2-50, qImage.height()/2, QString::fromUtf8("未 授 权"));
    }
}


void MarkView::draw_circle(){
    if(showCircle){QPainter painter(&paintQImage);
        painter.setPen(QColor(255,0,0));

        painter.drawLine(cx+0.5-20, cy+0.5, cx+0.5+20, cy+0.5);
        painter.drawLine(cx+0.5, cy+0.5-20, cx+0.5, cy+0.5+20);
        painter.drawArc(cx+0.5-r-1, cy+0.5-r-1,  2*r+0.5+1, 2*r+0.5+1, 0, 360*16);
    }
}

void MarkView::draw_focus_box(){
    if(showFocusBox){
        QPainter painter(&paintQImage);
        painter.setPen(QColor(255,0,0));
        painter.drawRect(focusBox.x, focusBox.y, focusBox.width, focusBox.height);
    }
}


void MarkView::paintEvent(QPaintEvent*){
    cvCvtColor(resizeImg, dispImg, CV_GRAY2RGB);
    draw_cross_line(paintQImage);
    draw_frame(paintQImage,patternFrame, QPen(QColor(0, 255, 0)));
    draw_frame(paintQImage,searchFrame, QPen(QColor(0, 0, 255)));
    //CircleFrame circleFrame(50,50,30);
    //draw_frame(paintQImage,circleFrame,QPen(QColor(255,0,0)));
    draw_status_text(paintQImage);
    draw_ccd_status(paintQImage);
    draw_diamond_pos(paintQImage);
    draw_circle();
    draw_focus_box();
    //draw_key_not_passed(paintQImage);
    QPainter painter(this);
    painter.drawImage(QPoint(0,0),paintQImage);
}

RectangleFrame::RectangleFrame(const Point& leftTop, double width, double height, double deg){
   double ang=deg/180.0*PI;
   conner[0]=leftTop;
   Vector2 vw(width, 0);
   Vector2 vh(0, height);
   vw=vector_rotate(vw, -ang);
   vh=vector_rotate(vh, -ang);
   conner[1]=leftTop;
   conner[1].move(vw);
   conner[3]=leftTop;
   conner[3].move(vh);
   conner[2]=conner[3];
   conner[2].move(vw);
   resizable=false;
   rotatable=false;
   visible=false;
}

void RectangleFrame::area_change(ChangeType type, double dx, double dy){

   switch(type){
       case left:
           left_move(dx, dy);
           break;
       case right:
           right_move(dx, dy);
           break;
       case top:
           top_move(dx, dy);
           break;
       case bottom:
           bottom_move(dx, dy);
           break;
       case lt:
           left_top_move(dx, dy);
           break;
       case lb:
           left_bottom_move(dx, dy);
           break;
       case rt:
           right_top_move(dx, dy);
           break;
       case rb:
           right_bottom_move(dx, dy);
           break;
       case MOVE:
           move(dx, dy);
           break;
       default:
           ;
   }
}

void RectangleFrame::left_move(double x, double y){
   Vector2 vm(x, y);
   Vector2 vn(conner[1], conner[0]);
   double vnLen=vn.length();
   double ip=inner_product(vm, vn);
   double ratio=ip/vnLen/vnLen;
   vn.zoom(ratio);
   conner[0].move(vn);
   conner[3].move(vn);
}

void RectangleFrame::right_move(double x, double y){
   Vector2 vm(x, y);
   Vector2 vn(conner[0], conner[1]);
   double vnLen=vn.length();
   double ip=inner_product(vm, vn);
   double ratio=ip/vnLen/vnLen;
   vn.zoom(ratio);
   conner[1].move(vn);
   conner[2].move(vn);
}

void RectangleFrame::top_move(double x, double y){
   Vector2 vm(x, y);
   Vector2 vn(conner[3], conner[0]);
   double vnLen=vn.length();
   double ip=inner_product(vm, vn);
   double ratio=ip/vnLen/vnLen;
   vn.zoom(ratio);
   conner[0].move(vn);
   conner[1].move(vn);
}
void RectangleFrame::bottom_move(double x, double y){
   Vector2 vm(x, y);
   Vector2 vn(conner[0], conner[3]);
   double vnLen=vn.length();
   double ip=inner_product(vm, vn);
   double ratio=ip/vnLen/vnLen;
   vn.zoom(ratio);
   conner[2].move(vn);
   conner[3].move(vn);
}

void RectangleFrame::conner_move(double x, double y, Vector2 & vnw, Vector2 & vnh){
   //double ratio=0;
   Vector2 vm(x, y);
   double vnwLen=vnw.length();
   double vnhLen=vnh.length();
   double ipw=inner_product(vm, vnw);
   double iph=inner_product(vm, vnh);
   double ratiow=ipw/vnwLen/vnwLen;
   double ratioh=iph/vnhLen/vnhLen;
   /*
   if(ratiow>0 && ratioh>0)
       ratio=MIN(ratiow, ratioh);
   else if(ratiow<0 && ratioh<0)
       ratio=MAX(ratiow, ratioh);
   else
       ratio=0;*/
   vnw.zoom(ratiow);
   vnh.zoom(ratioh);
}

void RectangleFrame::left_top_move(double x, double y){
   Vector2 vnw(conner[1], conner[0]);
   Vector2 vnh(conner[3], conner[0]);
   conner_move(x, y, vnw, vnh);
   conner[0].move(vnw);
   conner[3].move(vnw);
   conner[0].move(vnh);
   conner[1].move(vnh);
}

void RectangleFrame::left_bottom_move(double x, double y){
   Vector2 vnw(conner[2], conner[3]);
   Vector2 vnh(conner[0], conner[3]);
   conner_move(x, y, vnw, vnh);
   conner[3].move(vnw);
   conner[0].move(vnw);
   conner[3].move(vnh);
   conner[2].move(vnh);
}

void RectangleFrame::right_bottom_move(double x, double y){
   Vector2 vnw(conner[3], conner[2]);
   Vector2 vnh(conner[1], conner[2]);
   conner_move(x, y, vnw, vnh);
   conner[2].move(vnw);
   conner[1].move(vnw);
   conner[2].move(vnh);
   conner[3].move(vnh);
}

void RectangleFrame::right_top_move(double x, double y){
   Vector2 vnw(conner[0], conner[1]);
   Vector2 vnh(conner[2], conner[1]);
   conner_move(x, y, vnw, vnh);
   conner[1].move(vnw);
   conner[2].move(vnw);
   conner[1].move(vnh);
   conner[0].move(vnh);
}

void RectangleFrame::move(double x, double y){
   Vector2 vm(x,y);
   conner[0].move(vm);
   conner[1].move(vm);
   conner[2].move(vm);
   conner[3].move(vm);
}

const Point& RectangleFrame::get_conner(int index) const{
   assert(index>=0 && index<4);
   return conner[index];
}

Point RectangleFrame::get_center()const{
   double x= (conner[0].x()+conner[2].x())/2;
   double y= (conner[0].y()+conner[2].y())/2;
   return Point(x, y);
}

void RectangleFrame::rotate(double deg){
   Point mp((conner[0].x()+conner[2].x())/2, (conner[0].y()+conner[2].y())/2);
   double angle=deg/180.0*PI;
   for(int i=0; i<4; i++){
       Vector2 vmc(mp, conner[i]);
       vmc.rotate(-angle);
       conner[i].set(mp.x()+vmc.x(), mp.y()+vmc.y());
   }
}

void RectangleFrame::zoom(float widthRatio, float heightRatio){
   for(int i=0;i<4;i++)
       conner[i].set(conner[i].x()*widthRatio, conner[i].y()*heightRatio);
}

float RectangleFrame::get_width()const {
   float dx=conner[0].x()-conner[1].x();
   float dy=conner[0].y()-conner[1].y();
   return sqrt(dx*dx+dy*dy);
}

float RectangleFrame::get_height()const {
   float dx=conner[0].x()-conner[3].x();
   float dy=conner[0].y()-conner[3].y();
   return sqrt(dx*dx+dy*dy);
}

RectangleFrame::ChangeType RectangleFrame::get_ready_change_type(double x, double y, double range)const
{
   int i;
   Point pos(x, y);
   for(i=0;i<4;i++){
       if(two_points_distance(pos, conner[i])<range)
           return (ChangeType)(2*i);
       Line line(conner[i], conner[(i+1)%4]);
       if(two_points_distance(pos, mid_point(line))<range)
           return (ChangeType)(2*i+1);
   }

   Vector2 vect(get_center(), pos);
   Vector2 vectH(conner[0], conner[1]);
   Vector2 vectV(conner[0], conner[3]);
   float iph=fabs(inner_product(vect, vectH)/vectH.length());
   float ipv=fabs(inner_product(vect, vectV)/vectV.length());
   if(iph<get_width()/2+range && ipv<get_height()/2+range)
       return MOVE;

   return nochange;
}

CircleFrame::CircleFrame(double cx, double cy, double radius)
    :center(cx, cy), r(radius)
{
    visible=false;
    resizable=false;
    rotatable=false;
}

void CircleFrame::zoom(float ratio){
    center.set(center.x()*ratio, center.y()*ratio);
    r*=ratio;
}


Point CircleFrame::get_conner(int index)const{
    Point p(center);
    if(index==0)
        p.move(Vector2(0, -r));
    else if(index==1)
        p.move(Vector2(r, 0));
    else if(index==2)
        p.move(Vector2(0, r));
    else
        p.move(Vector2(-r, 0));
    return p;
}

Point CircleFrame::get_top_left()const{
    Point p(center);
    p.move(Vector2(-r, -r));
    return p;
}

CircleFrame::ChangeType CircleFrame::get_ready_change_type(double x, double y, double range)const{
    int i;
    double dis;
    Point mp(x, y);
    for(i=0;i<4;i++){
        dis=two_points_distance(get_conner(i), mp);
        if(dis<range){
            return (ChangeType)(top+2*i);
        }
    }
    dis=two_points_distance(mp, center);
    if(dis<r+range)
        return MOVE;
    return nochange;
}

void CircleFrame::area_change(ChangeType type, double dx, double dy){
    switch(type){
        case MOVE:
            move(dx, dy);
            break;
        case top:
            top_move(dx, dy);
            break;
        case bottom:
            bottom_move(dx, dy);
            break;
        case left:
            left_move(dx, dy);
            break;
        case right:
            right_move(dx, dy);
            break;
        default:
            ;
    }
}

void CircleFrame::move(double x, double y){
    center.move(Vector2(x, y));
}

void CircleFrame::top_move(double , double y){
    center.move(Vector2(0, y/2));
    r-=y/2;
}

void CircleFrame::bottom_move(double , double y){
    center.move(Vector2(0, y/2));
    r+=y/2;
}

void CircleFrame::left_move(double x, double ){
    center.move(Vector2(x/2, 0));
    r-=x/2;
}

void CircleFrame::right_move(double x, double ){
    center.move(Vector2(x/2, 0));
    r+=x/2;
}
