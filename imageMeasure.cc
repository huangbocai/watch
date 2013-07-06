/********************************************************************
* Description: imageMeasure.hh
* Definition for image measurement(not just for CVM), which contain image edge-recognizer,
* segment-synthesizer and geom-operater.
*
* Author: Huang Bocai, Chen Qiuqiang
* License:
* System: Linux
*
* Copyright (c) 2012 All rights reserved by Qiezhi.
********************************************************************/


#include <math.h>
#include <cassert>
#include <stdio.h>

#include "imageMeasure.hh"

static void edge_rel_pos(unsigned char *imgPoint, float *x, float *y, int step, int thresh);
bool  line_point_filter(PointsArray& points, float rmDis);
bool circle_point_filter(PointsArray& points, double rmDistance);
bool line_LSM(PointsArray& points, int& type, double& k, double& b, double maxDistance, long minPN);
bool line_LSM(PointsArray& points, int& type, Line& l, double maxDistance, long minPN);
bool circle_LSM(PointsArray& points, double& x0, double& y0, double& R, double rmDistance , long minPN);
bool circle_LSM(PointsArray& points, Circle& circle, double rmDistance , long minPN);
bool arc_LSM(PointsArray& points, Arc& arc, double rmDistance , long minPN);

static const char* cvCmdNames[]={"框选直线","框选圆","框选弧", "线选点","线选直线", "线选圆", "线选弧", "边缘切线"};
static const char* combCmdNames[]={"合成直线", "合成圆","合成圆弧"};
static const char* geomCmdNames[]={"两线交点","线段中点", "两点连线","中线","两点距离","点线距离","两线距离","两线夹角","中垂线", "两圆距离","圆线距离", "延长线", "圆线交点", "两圆切线", "两圆中线", "两圆交点"};

/*definition base figures's member function*/
GeomBase::GeomBase(double v0)
{
	dataNum=1;
	data[0]=v0;
}

GeomBase::GeomBase(double v0, double v1)
{
	dataNum=2;
	data[0]=v0;
	data[1]=v1;
}

GeomBase::GeomBase(double v0, double v1, double v2)
{
	dataNum=3;
	data[0]=v0;
	data[1]=v1;
	data[2]=v2;
}

GeomBase::GeomBase(double v0, double v1, double v2, double v3)
{
	dataNum=4;
	data[0]=v0;
	data[1]=v1;
	data[2]=v2;
	data[3]=v3;
}

GeomBase::GeomBase(double v0, double v1, double v2, double v3, double v4)
{
	dataNum=5;
	data[0]=v0;
	data[1]=v1;
	data[2]=v2;
	data[3]=v3;
	data[4]=v4;
}

double& GeomBase:: operator[](int i)
{
	assert(i<dataNum && i<GEOM_BASE_DATA_MAX);
	return data[i];
}

bool operator==(const GeomBase& gb1, const GeomBase& gb2){
	int i, num;
	if(gb1.dataNum!=gb2.dataNum)
		return false;
	for(i=0, num=gb1.dataNum; i<num; i++){
		if(!EQU(gb1.data[i], gb2.data[i]))
			return false;
	}
	return true;
}

bool operator!=(const GeomBase& gb1, const GeomBase& gb2){
	return !(gb1==gb2);
}


void Point::move(const Vector2 & v){
	data[0]+=v.x();
	data[1]+=v.y();
}


Vector2::Vector2(const Point & start, const Point & end){
	vx=end.x()-start.x();
	vy=end.y()-start.y();
}

double Vector2::length()const {
	return sqrt(vx*vx+vy*vy);
}

void Vector2::zoom(double ratio){
	vx*=ratio;
	vy*=ratio;
}

void Vector2::rotate(double angle){
	double x1=cos(angle)*vx-sin(angle)*vy;
	double y1=sin(angle)*vx+cos(angle)*vy;
	vx=x1;
	vy=y1;
}

double Line::length() const
{
	double dx=x1()-x2();
	double dy=y1()-y2();
	return sqrt(dx*dx+dy*dy);
}
void Line::move(const Vector2 & v){
	data[0]+=v.x();
	data[1]+=v.y();
	data[2]+=v.x();
	data[3]+=v.y();
}


Circle::Circle(const Point&  p1, const Point& p2, const Point& p3)
{
	dataNum=3;
	three_points_circle(p1, p2, p3, data[0], data[1], data[2]);
}
Circle::Circle(double x1, double y1, double x2, double y2, double x3 , double y3)
{
	Point p1(x1, y1);
	Point p2(x2, y2);
	Point p3(x3, y3); 
	dataNum=3;
	three_points_circle(p1, p2, p3, data[0], data[1], data[2]);
}
void Circle::set(double x1, double y1, double x2, double y2, double x3 , double y3)
{
	Point p1(x1, y1);
	Point p2(x2, y2);
	Point p3(x3, y3); 
	dataNum=3;
	three_points_circle(p1, p2, p3, data[0], data[1], data[2]);
}
void Circle::move(const Vector2 & v){
	data[0]+=v.x();
	data[1]+=v.y();
}

Arc::Arc(const Point & c, double r, double start, double end): GeomBase(c.x(), c.y(), r, start, end)
{
	data[3]=in_2pi(data[3]);
	data[4]=in_2pi(data[4]);
}
Arc::Arc(double cx, double cy, double r, double start, double end):GeomBase(cx, cy, r, start, end)
{
	data[3]=in_2pi(data[3]);
	data[4]=in_2pi(data[4]);
}
Arc::Arc(const Point&  p1, const Point& p2, const Point& p3)
{
	set(p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y());
}
Arc::Arc(double x1, double y1, double x2, double y2, double x3, double y3)
{
	set(x1, y1, x2,y2, x3, y3);
}
void Arc::set(double x1, double y1, double x2, double y2, double x3, double y3)
{
	Point p1(x1, y1);
	Point p2(x2, y2);
	Point p3(x3, y3); 
	dataNum=5;
	three_points_circle(p1, p2, p3, data[0], data[1], data[2]);
	double a1=arctan(x1-cx(), y1-cy());
	double a2=arctan(x2-cx(), y2-cy());
	double a3=arctan(x3-cx(), y3-cy());
	if((a2<a1 && a2<a3) ||(a2>a1 && a2>a3)){
		if(a1<a3){
			data[3]=a3;
			data[4]=a1;
		}
		else{
			data[3]=a1;
			data[4]=a3;
		}
	}
	else{
		if(a1<a3){
			data[3]=a1;
			data[4]=a3;
		}
		else{
			data[3]=a3;
			data[4]=a1;
		}
	}	
}
void Arc::move(const Vector2 & v){
	data[0]+=v.x();
	data[1]+=v.y();
}

GeomElement::GeomElement( bool valid):m_valid(valid)
{
	ptr= new GeomBase;
	assert (ptr);
	m_type= ANY;
}

GeomElement::GeomElement(const Point & point,  bool valid):m_valid(valid)
{
	ptr= new Point(point);
	assert (ptr);
	m_type= POINT;
}

GeomElement::GeomElement(const Line & line,  bool valid):m_valid(valid)
{	
	ptr= new Line(line);
	assert (ptr);
	m_type= LINE;
}

GeomElement::GeomElement(const Circle & circle,  bool valid):m_valid(valid)
{	
	ptr= new Circle(circle);
	assert (ptr);
	m_type= CIRCLE;
}

GeomElement::GeomElement(const Arc & arc , bool valid):m_valid(valid)
{	
	ptr= new Arc(arc);
	assert (ptr);
	m_type= ARC;
}

GeomElement::GeomElement(const Distance& distance,  bool valid):m_valid(valid)
{	
	ptr= new Distance(distance);
	assert (ptr);
	m_type= DISTANCE;
}

GeomElement::GeomElement(const Angle& angle , bool valid):m_valid(valid)
{	
	ptr= new Angle(angle);
	assert (ptr);
	m_type= ANGLE;
}

GeomElement::GeomElement(const GeomElement& ge)
{
	if(ge.m_type==POINT)
		ptr = new Point(*((Point*)ge.ptr));
	else if(ge.m_type==LINE)
		ptr = new Line(*((Line*)ge.ptr));
	else if(ge.m_type==CIRCLE)
		ptr = new Circle(*((Circle*)ge.ptr));
	else if(ge.m_type==ARC)
		ptr = new Arc(*((Arc*)ge.ptr));
	else if(ge.m_type==DISTANCE)
		ptr = new Distance(*((Distance*)ge.ptr));
	else if(ge.m_type==ANGLE)
		ptr = new Angle(*((Angle*)ge.ptr));
	else
		ptr= new GeomBase(*ge.ptr);
	assert (ptr);
	m_type= ge.m_type;
	m_valid=ge.m_valid;
}

GeomElement& GeomElement::operator=(const GeomElement& ge)
{
	if(this == &ge){
		return *this;
	}
	else{
		delete ptr;
		ptr=new GeomBase(*ge.ptr);
		assert (ptr);
		m_type= ge.m_type;
		m_valid= ge.m_valid;
		return *this;
	}
}

GeomElement::~GeomElement()
{
	assert (ptr);
	delete ptr;
	ptr=NULL;
}

const Point& GeomElement::get_point() const
{
	assert(m_type==POINT);
	Point* p= (Point*)ptr;
	return *p;
}

const Line& GeomElement::get_line() const
{
	assert(m_type==LINE);
	Line* p= (Line*)ptr;
	return *p;
}

const Circle& GeomElement::get_circle() const
{
	assert(m_type==CIRCLE);
	Circle* p= (Circle*)ptr;
	return *p;
}

const Arc& GeomElement::get_arc() const
{
	assert(m_type==ARC);
	Arc* p= (Arc*)ptr;
	return *p;	
}
const Distance& GeomElement::get_distance() const
{
	assert(m_type==DISTANCE);
	Distance* p=(Distance*)ptr;
	return *p;
}

const Angle& GeomElement::get_angle() const
{
	assert(m_type==ANGLE);
	Angle* p= (Angle*)ptr;
	return *p;
}

void GeomElement::move(const Vector2 & v){
	assert(m_type!=DISTANCE && m_type!=ANGLE);
	ptr->move(v);
}

/*difinition of image recognize selector*/
void Box::set(int  x1 , int y1, int x2, int y2){
	p1[0]=x1;
	p1[1]=y1;
	p2[0]=x2;
	p2[1]=y2;
}
int Box::left()const {
	if(p1.x()<p2.x()){
		return p1.x()+0.5;
	}
	else
		return p2.x()+0.5;
}
int Box::right()const{
	if(p1.x()>p2.x())
		return p1.x()+0.5;
	else
		return p2.x()+0.5;
}
int Box::top()const{
	if(p1.y()<p2.y())
		return p1.y()+0.5;
	else
		return p2.y()+0.5;
}
int Box::bottom()const {
	if(p1.y()>p2.y())
		return p1.y()+0.5;
	else
		return p2.y()+0.5;
}

BoxSelector::BoxSelector(int x1, int y1, int x2, int y2):m_box(x1, y1, x2,y2)
{
	set(x1, y1, x2, y2);
}
void BoxSelector::set(int x1, int y1, int x2, int y2)
{
	m_box.set( x1,  y1,  x2,  y2);
	if(fabs(x1-x2)>fabs(y1-y2)){
		if(y2>y1)
			m_direction=DOWN;
		else
			m_direction=UP;
	}
	else{
		if(x2>x1)
			m_direction=RIGHT;
		else
			m_direction=LEFT;
	}
}

void PointSelector::set(int x1, int y1, int x2, int y2){
	m_line[0]=x1;
	m_line[1]=y1;
	m_line[2]=x2;
	m_line[3]=y2;
}

void LineSelector::set(int x1, int y1, int x2, int y2, int width){
	m_line[0]=x1;
	m_line[1]=y1;
	m_line[2]=x2;
	m_line[3]=y2;
	m_width=width;
}

void CircleSelector::set(int x1, int y1, int x2, int y2, int x3, int y3, int lineWidth){
	m_width=lineWidth;
	m_circle.set(x1, y1, x2, y2, x3, y3);
}

void ArcSelector::set(int x1, int y1, int x2, int y2, int x3, int y3, int lineWidth){
	m_width=lineWidth;
	m_arc.set(x1, y1, x2, y2, x3,y3);
}

/*difinition image measure command*/

IMC::IMC(CmdType cmdType, const BoxSelector& box)
{
	assert(cmdType==LINE_IN_BOX || cmdType==CIRCLE_IN_BOX ||cmdType==ADJUST_CIRCLE_IN_BOX);
	m_cmdType=cmdType;
	selector=new BoxSelector(box);
}
IMC::IMC(const PointSelector& pointSelector){
	m_cmdType=POINT_IN_LINE;
	selector=new PointSelector(pointSelector);
}
IMC::IMC(const LineSelector& lineSelector){
	m_cmdType = LINE_IN_LINE;
	selector= new LineSelector(lineSelector);
}
IMC::IMC(const CircleSelector& circleSelector){
	m_cmdType = CIRCLE_IN_LINE;
	selector = new CircleSelector(circleSelector);
}
IMC::IMC(const ArcSelector& arcSelector){
	m_cmdType = ARC_IN_LINE;
	selector = new ArcSelector(arcSelector);
}
IMC::IMC(const LineSelector& lineSelector, int depend1):m_depend(1, depend1){
	m_cmdType= TANGENT_LINE;;
	selector= new LineSelector(lineSelector);
}
IMC::IMC(CmdType cmdType, int depend1):m_depend(1, depend1){
	assert (cmdType==MID_POINT ||cmdType==PERPENDICULAR_LINE);
	m_cmdType= cmdType;
	selector=NULL;
}

IMC::IMC(CmdType cmdType, int depend1, const Point& selPos1):m_depend(1, depend1){
	assert (cmdType==EXTEND_LINE);
	m_cmdType= cmdType;
	selPos[0]=selPos1;
	selector=NULL;
}

IMC::IMC(CmdType cmdType, int depend1, int depend2):m_depend(2, depend1){
	assert(cmdType==INTERSECTION_POINT ||cmdType==TWO_POINT_LINE || cmdType==TWO_POINT_DISTANCE
		||cmdType==POINT_LINE_DISTANCE ||cmdType==TWO_CIRCLE_DISTANCE
		||cmdType==CIRCLE_LINE_DISTANCE ||cmdType==TWO_CIRCLE_MID_LINE);
	m_cmdType=cmdType;
	m_depend[1]=depend2;
	selector=NULL;
}

IMC::IMC(CmdType cmdType, int depend1, int depend2, const Point& selPos1):m_depend(2, depend1){
	assert(  cmdType==CIRCLE_LINE_INTERSETION);
	m_cmdType=cmdType;
	selPos[0]=selPos1;
	m_depend[1]=depend2;
	selector=NULL;
}

IMC::IMC(CmdType cmdType, int depend1, int depend2, const Point& selPos1, const Point& selPos2)
	:m_depend(2, depend1){
	assert(  cmdType==TWO_LINE_DISTANCE ||cmdType==TWO_LINE_ANGLE ||cmdType == MID_LINE 
		||cmdType==TWO_CIRCLE_INTERSETION ||cmdType==TWO_CIRCLE_TANGENT);
	m_cmdType=cmdType;
	selPos[0]=selPos1;
	selPos[1]=selPos2;
	m_depend[1]=depend2;
	selector=NULL;
}

IMC::IMC(CmdType cmdType, const vector<int>& depends):m_depend(depends){
	assert(IMC::is_comb_cmd(cmdType) || IMC:: is_geom_cmd(cmdType));
	m_cmdType= cmdType;
	selector=NULL;
}

IMC::IMC(const IMC& imc):m_cmdType(imc.m_cmdType), m_depend(imc.m_depend)
{
	selPos[0]=imc.selPos[0];
	selPos[1]=imc.selPos[1];
	if(imc.selector){
		if(m_cmdType==LINE_IN_BOX || m_cmdType==CIRCLE_IN_BOX ||m_cmdType==ADJUST_CIRCLE_IN_BOX){
			BoxSelector* p= (BoxSelector*) imc.selector;
			selector= new BoxSelector(*p);
		}
		else if(m_cmdType==POINT_IN_LINE){
			PointSelector* p =(PointSelector*)imc.selector;
			selector = new PointSelector(*p);
		}
		else if(m_cmdType==LINE_IN_LINE ||m_cmdType==TANGENT_LINE){
			LineSelector* p= (LineSelector*)imc.selector;
			selector = new LineSelector(*p);
		}
		else if(m_cmdType==CIRCLE_IN_LINE){
			CircleSelector* p= (CircleSelector*)imc.selector;
			selector = new CircleSelector(*p);
		}
		else if(m_cmdType==ARC_IN_LINE){
			ArcSelector* p= (ArcSelector*)imc.selector;
			selector = new ArcSelector(*p);
		}
		else
			assert(0);
	}
	else
		selector=NULL;
}
const BoxSelector& IMC::box_selector() const{
	assert(m_cmdType==LINE_IN_BOX ||m_cmdType==CIRCLE_IN_BOX ||m_cmdType==ADJUST_CIRCLE_IN_BOX);
	assert(selector);
	BoxSelector* p=(BoxSelector*)selector;
	return *p;
}

const PointSelector& IMC::point_selector() const{
	assert(m_cmdType==POINT_IN_LINE);
	assert(selector);
	PointSelector* p=(PointSelector* )selector;
	return *p;
}
const LineSelector& IMC::line_selector() const{
	assert(m_cmdType==LINE_IN_LINE ||m_cmdType==TANGENT_LINE);
	assert(selector);
	LineSelector* p=(LineSelector*)selector;
	return *p;
}
const CircleSelector& IMC::circle_selector() const{
	assert(m_cmdType==CIRCLE_IN_LINE);
	assert(selector);
	CircleSelector* p=(CircleSelector*)selector;
	return *p;
}
const ArcSelector& IMC::arc_selector() const{
	assert(m_cmdType==ARC_IN_LINE);
	assert(selector);
	ArcSelector* p=(ArcSelector*)selector;
	return *p;
}

const char* IMC::cmd_name(CmdType type)
{
	if(is_cv_cmd(type)){
		return cvCmdNames[type-LINE_IN_BOX];
	}
	else if(is_comb_cmd(type)){
		return combCmdNames[type-COMB_LINE];
	}
	else if(is_geom_cmd(type)){
		return geomCmdNames[type-INTERSECTION_POINT];
	}
	else
		assert(0);
}

bool IMC::is_cv_cmd(CmdType type){
	return int(type)>=int(LINE_IN_BOX) && int(type)<int(COMB_LINE);
}
bool IMC::is_comb_cmd(CmdType type){
	return int(type)>=int(COMB_LINE) && int(type)<int(INTERSECTION_POINT);
}
bool IMC::is_geom_cmd(CmdType type){
	return int(type)>=int(INTERSECTION_POINT) && int(type)<int(MEASURE_END);
}

GeomElement::GeomType IMC::result_type(CmdType type){
	switch(type){
		case POINT_IN_LINE:
		case INTERSECTION_POINT:
		case MID_POINT:
		case CIRCLE_LINE_INTERSETION:
		case TWO_CIRCLE_INTERSETION:
			return GeomElement::POINT;
		case LINE_IN_BOX:
		case LINE_IN_LINE:
		case TANGENT_LINE:
		case COMB_LINE:
		case TWO_POINT_LINE:
		case MID_LINE:
		case PERPENDICULAR_LINE:
		case EXTEND_LINE:
		case TWO_CIRCLE_TANGENT:
		case TWO_CIRCLE_MID_LINE:
			return GeomElement::LINE;
		case CIRCLE_IN_BOX:
		case CIRCLE_IN_LINE:
		case COMB_CIRCLE:
			return GeomElement::CIRCLE;
		case ARC_IN_BOX:
		case ARC_IN_LINE:
		case COMB_ARC:
			return GeomElement::ARC;
		case TWO_POINT_DISTANCE:
		case POINT_LINE_DISTANCE:
		case TWO_LINE_DISTANCE:
		case TWO_CIRCLE_DISTANCE:
		case CIRCLE_LINE_DISTANCE:
			return GeomElement::DISTANCE;
		case TWO_LINE_ANGLE:
			return GeomElement::ANGLE;
		default:
			assert(0);
	}
}

const IMC& IMC::operator=(const IMC& imc){
	if(this==&imc)
		return *this;
	m_cmdType=imc.m_cmdType;
	m_depend=imc.m_depend;
	selPos[0]=imc.selPos[0];
	selPos[1]=imc.selPos[1];
	delete_selector();
	if(imc.selector){
		if(m_cmdType==LINE_IN_BOX || m_cmdType==CIRCLE_IN_BOX ||m_cmdType==ADJUST_CIRCLE_IN_BOX){
			BoxSelector* p= (BoxSelector*) imc.selector;
			selector= new BoxSelector(*p);
		}
		else if(m_cmdType==POINT_IN_LINE){
			PointSelector* p =(PointSelector*)imc.selector;
			selector = new PointSelector(*p);
		}
		else if(m_cmdType==LINE_IN_LINE ||m_cmdType==TANGENT_LINE){
			LineSelector* p= (LineSelector*)imc.selector;
			selector = new LineSelector(*p);
		}
		else if(m_cmdType==CIRCLE_IN_LINE){
			CircleSelector* p= (CircleSelector*)imc.selector;
			selector = new CircleSelector(*p);
		}
		else if(m_cmdType==ARC_IN_LINE){
			ArcSelector* p= (ArcSelector*)imc.selector;
			selector = new ArcSelector(*p);
		}
		else
			assert(0);
	}
	else
		selector=NULL;
	return *this;
}

void IMC::delete_selector()
{
	if (selector){
		if(m_cmdType==LINE_IN_BOX ||m_cmdType==CIRCLE_IN_BOX ||
			m_cmdType==ADJUST_CIRCLE_IN_BOX)
			delete (BoxSelector*)selector;
		else if(m_cmdType==POINT_IN_LINE)
			delete (PointSelector*)selector;
		else if(m_cmdType==LINE_IN_LINE ||m_cmdType==TANGENT_LINE)
			delete (LineSelector*)selector;
		else if(m_cmdType==CIRCLE_IN_LINE)
			delete (CircleSelector*)selector;
		else if(m_cmdType==ARC_IN_LINE)
			delete (ArcSelector*)selector;
		else
			assert("new error");
		selector=NULL;
	}
}

IMC::~IMC(){
	delete_selector();
}


TransformMatrix::TransformMatrix(double kxx, double kxy, double kyx, double kyy)
	:m_kxx(kxx),m_kxy(kxy),m_kyx(kyx),m_kyy(kyy)
{
	double det=m_kxx*m_kyy-m_kxy*m_kyx;
	assert(det>0.0000000001 || det<-0.0000000001);
	in_kxx=m_kyy/det;
	in_kxy=-m_kxy/det;
	in_kyx=-m_kyx/det;
	in_kyy=m_kxx/det;
	rotateAngle=(arctan(m_kyy, -m_kxy)+arctan(m_kxx, m_kyx))/2;
}
void TransformMatrix::transform(double srcx, double srcy, double& detx, double& dety) const
{
	detx=m_kxx*srcx+m_kxy*srcy;
	dety=m_kyx*srcx+m_kyy*srcy;
}
void  TransformMatrix::inv_transform(double srcx, double srcy, double &detx, double& dety) const
{
	detx=in_kxx*srcx+in_kxy*srcy;
	dety=in_kyx*srcx+in_kyy*srcy;
}


ImageActualTM& ImageActualTM::operator=(const TransformMatrix& tm){
	TransformMatrix::operator=(tm);
	return *this;
}

Point ImageActualTM::transform(double imgX, double imgY)const
{
	double actx,acty; 
	TransformMatrix::transform(imgX-halfWidth, halfHeight-imgY, actx, acty);
	return Point(actx+cx, acty+cy);
}
Point ImageActualTM::inv_transform(const Point & actual)const
{
	double imgx, imgy;
	TransformMatrix::inv_transform(actual.x()-cx, actual.y()-cy, imgx, imgy);
	return Point(imgx+halfWidth, halfHeight-imgy);
}
Point ImageActualTM::inv_transform(double x, double y)const
{
	double imgx, imgy;
	TransformMatrix::inv_transform(x-cx, y-cy, imgx, imgy);
	return Point(imgx+halfWidth, halfHeight-imgy);
}

PointsArray::PointsArray(const PointsArray & pa){
	ps.reserve(pa.valid_num());
	vector<ValidPoint>::const_iterator it;
	for(it=pa.ps.begin();it!=pa.ps.end();it++){
		if(it->valid)
			ps.push_back(*it);
	}
}
void PointsArray::add(double x, double y)
{
	ps.push_back(ValidPoint(x,y));
}

void PointsArray::add(const Point& point){
	ps.push_back(ValidPoint(point.x(), point.y()));
}

void PointsArray::add(const ValidPoint& point){
	ps.push_back(point);
}

void PointsArray::add(const PointsArray& addend)
{
	vector<ValidPoint>::const_iterator it;
	const vector<ValidPoint>& addPs=addend.ps;
	for(it=addPs.begin(); it!=addPs.end(); it++){
		if(it->valid)
			ps.push_back(*it);
	}
}

void PointsArray::sample(unsigned int sampleNum){
	unsigned int validNum=valid_num();
	if(validNum<sampleNum)
		return;
	double rate=1.0*validNum/sampleNum;
	unsigned int sampleN=0;
	unsigned int nowN=-1;
	unsigned int askN=(int)(rate* sampleN);
	vector<ValidPoint>::iterator it;
	for(it=ps.begin();it!=ps.end();it++){
		if(it->valid){
			nowN++;
			if(nowN!=askN)
				it->valid=false;
			else{
				sampleN++;
				askN=rate* sampleN;
			}
		}
		if(sampleN==sampleNum){
			it++;
			break;
		}
	}
	for(;it!=ps.end();it++)
		it->valid=false;
}


int PointsArray::valid_num() const
{
	int vn=0;
	vector<ValidPoint>::const_iterator it;
	for(it=ps.begin();it!=ps.end();it++){
		if(it->valid)
			vn++;
	}
	return vn;
}


void PointsArray::move(const Vector2 & v){
	double vx=v.x();
	double vy=v.y();
	vector<ValidPoint>::iterator it;
	for(it=ps.begin();it!=ps.end();it++){
		it->x+=vx;
		it->y+=vy;	
	}
}

void PointsArray::valid_only(){
	vector<ValidPoint>::iterator it, validIt;
	for(it=ps.begin(), validIt=ps.begin(); it!=ps.end(); it++){
		if(it->valid){
			*validIt=*it;
			validIt++;
		}
	}
	ps.erase(validIt, ps.end());
}

const ValidPoint& PointsArray::first_valid() const{
	vector<ValidPoint>::const_iterator it;
	for(it=ps.begin();it!=ps.end();it++){
		if(it->valid)
			break;
	}
	return *it;
}

const ValidPoint& PointsArray::last_valid() const{
	vector<ValidPoint>::const_reverse_iterator it;
	for(it=ps.rbegin();it!=ps.rend();it++){
		if(it->valid)
			break;
	}
	return *it;
}

EdgeRecognizer::EdgeRecognizer(const TransformMatrix& transformMatrix, float roughRmDistance,
	float lsmRmDistance, int gradientThresh, int grayThresh)
	:smooth(NULL), tm(transformMatrix), points(400), gradientTh(gradientThresh), grayTh(grayThresh),roughRmDis(roughRmDistance), lsmRmDis(lsmRmDistance){}

EdgeRecognizer::~EdgeRecognizer(){
	if(smooth)
		cvReleaseImage(&smooth);
}

GeomElement EdgeRecognizer::recognize(const IplImage* image, const IMC& imc, const GeomElement& ge){
	//must be gray image
	assert(image->depth==IPL_DEPTH_8U && image->nChannels==1);
	//allocate smooth if image size changed
	if(smooth==NULL)
		smooth=cvCreateImage(cvSize(image->width, image->height),IPL_DEPTH_8U, 1);
	else if(image->width!=smooth->width || image->height!=smooth->height){
		cvReleaseImage(&smooth);
		smooth=cvCreateImage(cvSize(image->width, image->height),IPL_DEPTH_8U, 1);
	}
	
	//get and smooth the image
	cvSmooth(image, smooth, CV_MEDIAN, 3, 0, 0, 0);

	//recognize edge according to imc
	IMC::CmdType cmdType=imc.cmd_type();
	bool sucess;
	
	assert(IMC::is_cv_cmd(cmdType) ||cmdType==IMC::ADJUST_CIRCLE_IN_BOX);
	if(cmdType== IMC::LINE_IN_BOX){
		Line line=line_in_box(imc.box_selector(), sucess);
		return GeomElement(line, sucess);
	}
	else if(cmdType==IMC::CIRCLE_IN_BOX ||cmdType==IMC::ADJUST_CIRCLE_IN_BOX){
		Circle circle=circle_in_box(imc.box_selector(), sucess);
		return GeomElement(circle, sucess);
	}
	else if(cmdType==IMC::POINT_IN_LINE){
		Point point = point_in_line(imc.point_selector(), sucess);
		return GeomElement(point, sucess);
	}
	else if(cmdType==IMC::LINE_IN_LINE){
		Line line= line_in_line(imc.line_selector(), sucess);
		return GeomElement(line, sucess);
	}
	else if(cmdType==IMC::CIRCLE_IN_LINE){
		Circle circle=circle_in_line(imc.circle_selector(), sucess);
		return GeomElement(circle, sucess);
	}
	else if(cmdType==IMC::ARC_IN_LINE){
		Arc arc=arc_in_line(imc.arc_selector(), sucess);
		return GeomElement(arc, sucess);
	}
	else if(cmdType==IMC::TANGENT_LINE){
		if(!ge.valid())
			return  GeomElement(Line(),false);
		Line line=tangent_line(imc.line_selector(), ge.get_line(), sucess);
		return GeomElement(line,sucess);
	}
	else
		return GeomElement();
}

Line EdgeRecognizer::line_in_box(const BoxSelector& boxSltr, bool& sucess)
{
	int type;
	Line l;
	scan_edge_in_box_selector(boxSltr);
	sucess=line_point_filter(points, roughRmDis);
	if(!sucess)
		return l;
	points.sample(maxPoints);
	const ValidPoint& first=points.first_valid();
	const ValidPoint& last=points.last_valid();
	double dx=first.x-last.x;
	double dy=first.y-last.y;
	if(fabs(dx)>fabs(dy))
		type=Y_EQU_FX;
	else
		type=X_EQU_FY;
	sucess=line_LSM(points, type, l, lsmRmDis, 10);
	return l;
}

Circle EdgeRecognizer::circle_in_box(const BoxSelector& boxSltr, bool& sucess)
{
	Circle circle;
	circle_edge_in_box_selector(boxSltr);
	sucess=circle_point_filter( points, roughRmDis);
	if(!sucess)
		return circle;
	points.sample(maxPoints);
	sucess=circle_LSM(points, circle, lsmRmDis, 10);
	return circle;
}

Point EdgeRecognizer::point_in_line(const PointSelector& pointSltr, bool& sucess)
{
	double px=0, py=0;
	//clear formerly points
	points.clear();
	sucess=scan_no_stain_edge_in_line(pointSltr, px, py);
	points.add(px, py);
	return Point(px, py);
}

Line EdgeRecognizer::line_in_line(const LineSelector& lineSltr, bool& sucess)
{
	int type;
	Line l;
	scan_edge_in_line_selector(lineSltr);
	sucess=line_point_filter(points, roughRmDis);
	if(!sucess)
		return l;
	points.sample(maxPoints);
	const ValidPoint& first=points.first_valid();
	const ValidPoint& last=points.last_valid();
	double dx=first.x-last.x;
	double dy=first.y-last.y;
	if(fabs(dx)>fabs(dy))
		type=Y_EQU_FX;
	else
		type=X_EQU_FY;
	sucess=line_LSM(points, type, l, lsmRmDis, 10);
	return l;
}

Line EdgeRecognizer::tangent_line(const LineSelector& lineSltr, const Line& parallel, bool& sucess)
{
	//scan edge points
	scan_edge_in_line_selector(lineSltr, 4);

	//figure up reference line
	int halfWidth=smooth->width/2;
	int halfHeight=smooth->height/2;
	int x1=lineSltr.x1(), y1=lineSltr.y1(), x2=lineSltr.x2(), y2=lineSltr.y2();
	int vx=x2-x1;
	int vy=y2-y1;
	if(vx==0 && vy==0){
		sucess=false;
		return Line();
	}
	float len=sqrt(vx*vx+vy*vy);
	Vector2 v(vx, vy);
	Vector2 v1=vector_rotate(v, 0.5*PI);
	float dx=v1.x()/len*1000;
	float dy=v1.y()/len*1000;
	double passx, passy;
	if(lineSltr.get_direction()==LineSelector::LEFT)
		tm.transform(x1+dx-halfWidth, halfHeight-(y1+dy), passx, passy);
	else
		tm.transform(x1-dx-halfWidth, halfHeight-(y1-dy), passx, passy);
	Point midPoint=mid_point(parallel);
	Vector2 moveVect(passx-midPoint.x(), passy-midPoint.y());
	Line referLine=parallel;
	referLine.move(moveVect);

	//caculate the distances from edge points to reference line
	vector<ValidPoint>::iterator it;
	vector<double> disArray;
	double dis;
	for(it=points.begin(); it!=points.end(); it++){
		Point p(it->x, it->y);
		dis=point_line_distance(p, referLine);
		disArray.push_back(dis);
	}

	//find the first 1/5 neareast points
	int pointsSize=disArray.size();
	if(pointsSize==0){
		sucess=false;
		return Line();
	}
	int num=pointsSize/3;
	if(num>100)
		num=100;
	else if(num<10){
		if(pointsSize<10)
			num=pointsSize;
		else
			num=10;
	}
	int i,j, minIndex;
	double minVal, sum=0;
	for(i=0;i<num;i++){
		minIndex=i;
		minVal=disArray[i];
		for(j=i+1;j<pointsSize;j++){
			if(disArray[j]< minVal){
				minVal=disArray[j];
				minIndex=j;
			}
		}
		if(minIndex!=i){
			disArray[minIndex]=disArray[i];
			disArray[i]=minVal;
		}
		sum+=minVal;
	}

	//caculate the satatic data
	printf("sum=%lf, num=%d\n", sum, num);
	double averge=sum/num;
	sum=0;
	for(i=0;i<num;i++)
		sum+=(disArray[i]-averge)*(disArray[i]-averge);
	double standarDiff=sqrt(sum/num);
	printf("avg=%f, SD=%f\n", averge, standarDiff);
	double rangeRight=averge+1.5*standarDiff;
	double rangeLeft=averge-1.5*standarDiff;

	//find the nears point 
	vector<ValidPoint>::iterator nearIt;
	double minDis=rangeRight;	
	bool nearFlag=false;
	for(it=points.begin(); it!=points.end(); it++){
		Point p(it->x, it->y);
		dis=point_line_distance(p, referLine);
		if(dis>rangeLeft && dis<rangeRight){
			if(dis<minDis){
				nearFlag=true;
				minDis=dis;
				nearIt=it;
			}
		}
	}
	if(!nearFlag){
		sucess=false;
		return Line();
	}

	//
	//printf("near x: %lf, near y: %lf\n", nearIt->x, nearIt->y);
	moveVect.set(nearIt->x-midPoint.x(), nearIt->y-midPoint.y());
	Line tangLine=parallel;
	tangLine.move(moveVect);
	sucess=true;
	points.clear();
	points.add(tangLine.x1(), tangLine.y1());
	points.add(tangLine.x2(), tangLine.y2());
	return tangLine;
	
}

Circle EdgeRecognizer::circle_in_line(const CircleSelector& circleSltr, bool& sucess)
{
	Circle circle;
	scan_edge_in_circle_selector(circleSltr);
	sucess=circle_point_filter(points, roughRmDis);
	if(!sucess)
		return Circle();
	points.sample(maxPoints);
	sucess=circle_LSM(points, circle, lsmRmDis, 10);
	return circle;
}

Arc EdgeRecognizer::arc_in_line(const ArcSelector& arcSltr, bool& sucess)
{
	Arc arc;
	scan_edge_in_arc_selector(arcSltr);
	//printf("scanf %d points\n",points.valid_num());
	//printf("rough rm %f\n", roughRmDis);
	sucess=circle_point_filter(points,roughRmDis);
	//printf("after filter %d points\n",points.valid_num());
	if(!sucess)
		return arc;
	points.sample(maxPoints);
	//printf("lsm rm %f\n", lsmRmDis);
	sucess=arc_LSM(points, arc, lsmRmDis, 10);
	//printf("lsm %d points\n",points.valid_num());
	return arc;
}

bool EdgeRecognizer::scan_edge_in_line(const PointSelector & pointSltr, double& px, double& py)
{
	int x1=pointSltr.x1(), x2=pointSltr.x2(), y1=pointSltr.y1(), y2=pointSltr.y2();
	int width_1=smooth->width-1, height_1=smooth->height-1;
	if(x1<0 ||x2<0 ||y1<0 ||y2<0 ||x1>width_1 ||x2>width_1 ||y1>height_1 ||y2>height_1)
		return false;
	
	if(abs(x1-x2)<3 && abs(y1-y2)<3)
		return false;
	
	float colFloat, rowFloat, colIncr, rowIncr;
	int col,row, nextCol, nextRow, edgeCol=0, edgeRow=0;
	int widthStep=smooth->widthStep;
	int gradient, maxGrad;
	unsigned char *prePoint, *nowPoint, *nextPoint, *edgePoint=NULL; 
	unsigned char* imageData=(unsigned char*)smooth->imageData;
	bool intoEdgeArea=false;
	float dx, dy;
	int halfWidth=smooth->width/2;
	int halfHeight=smooth->height/2;
	
	//the increase of row and collumn
	if(abs(x2-x1)>=abs(y2-y1)){
		colIncr=1.0*(x2-x1)/abs(x2-x1);
		rowIncr=1.0*(y2-y1)/abs(x2-x1);
	}
	else{
		colIncr=1.0*(x2-x1)/abs(y2-y1);
		rowIncr=1.0*(y2-y1)/abs(y2-y1);
	}
	//previous point
	prePoint=imageData+y1*widthStep+x1;
	//current point
	colFloat=x1+colIncr; rowFloat=y1+rowIncr;
	col=colFloat+0.5; row=rowFloat+0.5;
	nowPoint=imageData+row*widthStep+col;
	//next point
	colFloat+=colIncr; rowFloat+=rowIncr;
	nextCol=colFloat+0.5; nextRow=rowFloat+0.5;
	nextPoint=imageData+nextRow*widthStep+nextCol;

	//scan the edge point
	maxGrad=gradientTh;
	while(nextCol!=x2 ||nextRow!=y2){
		gradient=abs(*nextPoint -*prePoint);
		if(gradient>gradientTh){
			intoEdgeArea=true;
			if(gradient>maxGrad){
				maxGrad=gradient;
				edgeCol=col;
				edgeRow=row;
				edgePoint=nowPoint;
			}
		}
		else{
			if(intoEdgeArea)
				break;
		}
		//prepare for next time
		prePoint=nowPoint;
		nowPoint=nextPoint;
		col=nextCol;  row=nextRow;
		colFloat+=colIncr; rowFloat+=rowIncr;
		nextCol=colFloat+0.5; nextRow=rowFloat+0.5;
		//next point pointer increase
		if(nextCol>col)
			nextPoint++;
		else if(nextCol<col)
			nextPoint--;
		if(nextRow>row)
			nextPoint+=widthStep;
		else if(nextRow<row)
			nextPoint-=widthStep;
	}

	//whether find the edge point?
	if(maxGrad>gradientTh){
		edge_rel_pos(edgePoint, &dx, &dy, widthStep, grayTh);
		tm.transform(edgeCol+dx-halfWidth, halfHeight-(edgeRow+dy), px, py);
		if(fabs(px)>10 ||fabs(py)>10)
			printf("in line (%d, %d)->(%d, %d), x=%f, y=%f\n", x1, y1, x2, y2, px, py);
		return true;
	}
	else
		return false;
}

void EdgeRecognizer::scan_edge_in_box_selector(const BoxSelector& boxSltr)
{
	int i,j;
	int *row, *col;
	int intoEdgeArea, maxGrad;
	int edgeRow=0, edgeCol=0;
	unsigned char* edgePoint=NULL;
	int gradient;
	float dx=0, dy=0;
	double px, py;	
	unsigned char* imgData=(unsigned char*)smooth->imageData;
	int widthStep=smooth->widthStep;
	int halfWidth=smooth->width/2;
	int halfHeight=smooth->height/2;
	
	//get box selector boundary and direction
	int bt=boxSltr.top();
	int bb=boxSltr.bottom();
	int bl=boxSltr.left();
	int br=boxSltr.right();
	BoxSelector::Direction dir=boxSltr.get_direction();
	assert(bt>=0 && bb<smooth->height && bl>=0 && br<smooth->width);
	
	//clear formerly points
	points.clear();
	
	//determinate scan way by box selector direction
	unsigned char  *lineIt, *pointIt;//iterator of line and point
	int lineItIncr, pointItIncr;
	int lineBegin, lineEnd,lineIncr, pointBegin, pointEnd, pointIncr;
	if(dir==BoxSelector::RIGHT){
		lineIt=imgData+bt*widthStep+bl;
		lineItIncr=widthStep; pointItIncr=1;
		lineBegin=bt; lineEnd=bb+1;lineIncr=1;
		pointBegin=bl; pointEnd=br+1; pointIncr=1;
		row=&i; col=&j;
	}
	else if(dir==BoxSelector::LEFT){
		lineIt=imgData+bt*widthStep+br;
		lineItIncr=widthStep; pointItIncr=-1;
		lineBegin=bt; lineEnd=bb+1; lineIncr=1;
		pointBegin=br; pointEnd=bl-1; pointIncr=-1;
		row=&i; col=&j;
	}
	else if(dir==BoxSelector::DOWN){
		lineIt=imgData+bt*widthStep+bl;
		lineItIncr=1; pointItIncr=widthStep;
		lineBegin=bl; lineEnd=br+1;lineIncr=1;
		pointBegin=bt; pointEnd=bb+1; pointIncr=1;
		row=&j; col=&i;
	}
	else{
		lineIt=imgData+bb*widthStep+bl;
		lineItIncr=1; pointItIncr=-widthStep;
		lineBegin=bl; lineEnd=br+1;lineIncr=1;
		pointBegin=bb; pointEnd=bt-1;pointIncr=-1;
		row=&j; col=&i;
	}
	
	//scan the edge points and save them
	for(i=lineBegin; i!=lineEnd; i+=lineIncr, lineIt+=lineItIncr){//next line
		intoEdgeArea=0;
		maxGrad=gradientTh;
		for(pointIt=lineIt, j=pointBegin; j!=pointEnd; j+=pointIncr, pointIt+=pointItIncr){//next point
			gradient=abs(pointIt[pointItIncr] -pointIt[-pointItIncr]) ;
			if(gradient>gradientTh){
				intoEdgeArea=1;
				if(gradient>maxGrad){
					maxGrad=gradient;
					edgeRow=*row;
					edgeCol=*col;
					edgePoint=pointIt;
				}
			}
			else if(intoEdgeArea) //already get the edge point
				break;
		}
		if(maxGrad>gradientTh){
			edge_rel_pos(edgePoint, &dx, &dy, widthStep, grayTh);
			tm.transform(edgeCol+dx-halfWidth, halfHeight-(edgeRow+dy), px, py);
			points.add(px, py);
		}
	}
}

void  EdgeRecognizer::circle_edge_in_box_selector(const BoxSelector& boxSltr)
{
	int row, col;
	float dx,dy;
	double px, py;
	unsigned char *imgData, *lineIt, *pointIt;
	int gradient;
	int thresh=2*gradientTh;
	int halfWidth=smooth->width/2;
	int halfHeight=smooth->height/2;

	//get box selector boundary 
	int bt=boxSltr.top();
	int bb=boxSltr.bottom();
	int bl=boxSltr.left();
	int br=boxSltr.right();
	assert(bt>=0 && bb<smooth->height && bl>=0 && br<smooth->width);

	int widthStep=smooth->widthStep;
	imgData=(unsigned char*)smooth->imageData;
	lineIt=imgData+(bt+1)*widthStep+(bl+1);

	//clear formerly points
	points.clear();

	//scan the edge points and save them
	for(row=bt+1;row<bb;row++)
	{
		pointIt=lineIt;
		for(col=bl+1;col<br;col++)
		{
			gradient=abs(pointIt[1]-pointIt[-1]) + abs(pointIt[widthStep]-pointIt[-widthStep]);
			if(gradient>thresh)
			{
				edge_rel_pos(pointIt, &dx, &dy, widthStep, grayTh);
				tm.transform(col+dx-halfWidth, halfHeight-(row+dy), px, py);
				points.add(px, py);
			}
			pointIt++;
		}
		lineIt+=widthStep;
	}
}


bool EdgeRecognizer::scan_no_stain_edge_in_line(const PointSelector & pointSltr, double& px, double&py, int stainSize)
{
	int x1=pointSltr.x1(), x2=pointSltr.x2(), y1=pointSltr.y1(), y2=pointSltr.y2();
	int width_1=smooth->width-1, height_1=smooth->height-1;
	if(x1<0 ||x2<0 ||y1<0 ||y2<0 ||x1>width_1 ||x2>width_1 ||y1>height_1 ||y2>height_1)
		return false;
	if(abs(x1-x2)<3 && abs(y1-y2)<3)
		return false;
	
	float colFloat, rowFloat, colIncr, rowIncr;
	int col,row, nextCol, nextRow, edgeCol=0, edgeRow=0;
	int widthStep=smooth->widthStep;
	int gradient, maxGrad;
	unsigned char *prePoint, *nowPoint, *nextPoint, *edgePoint=NULL; 
	unsigned char* imageData=(unsigned char*)smooth->imageData;
	bool intoEdgeArea=false;
	float dx, dy;
	int halfWidth=smooth->width/2;
	int halfHeight=smooth->height/2;
	enum {foreFlat, scanEdge, backFlat, scanDone} scanState=foreFlat;
	int flatSum=0;
	
	//the increase of row and collumn
	if(abs(x2-x1)>=abs(y2-y1)){
		colIncr=1.0*(x2-x1)/abs(x2-x1);
		rowIncr=1.0*(y2-y1)/abs(x2-x1);
	}
	else{
		colIncr=1.0*(x2-x1)/abs(y2-y1);
		rowIncr=1.0*(y2-y1)/abs(y2-y1);
	}
	//previous point
	prePoint=imageData+y1*widthStep+x1;
	//current point
	colFloat=x1+colIncr; rowFloat=y1+rowIncr;
	col=colFloat+0.5; row=rowFloat+0.5;
	nowPoint=imageData+row*widthStep+col;
	//next point
	colFloat+=colIncr; rowFloat+=rowIncr;
	nextCol=colFloat+0.5; nextRow=rowFloat+0.5;
	nextPoint=imageData+nextRow*widthStep+nextCol;

	//scan the edge point
	maxGrad=gradientTh;
	while(nextCol!=x2 ||nextRow!=y2){
		gradient=abs(*nextPoint -*prePoint);
		//find the flat segment befor edge
		if(scanState==foreFlat){
			if(gradient<gradientTh){
				flatSum++;
				if(flatSum>stainSize){
					intoEdgeArea=false;
					maxGrad=gradientTh;
					scanState=scanEdge;
				}
			}
			else
				flatSum=0;
		}
		//find the max gradient edge point
		else if(scanState==scanEdge){
			if(gradient>gradientTh){
				intoEdgeArea=true;
				if(gradient>maxGrad){
					maxGrad=gradient;
					edgeCol=col;
					edgeRow=row;
					edgePoint=nowPoint;
				}
			}
			else{
				if(intoEdgeArea){
					flatSum=0;
					scanState=backFlat;
				}
			}
		}
		else if(scanState==backFlat){
			if(gradient<gradientTh){
				flatSum++;
				if(flatSum>stainSize){
					scanState=scanDone;
					break;
				}
			}
			else{
				flatSum=0;
				scanState=foreFlat;
			}
		}
		else
			assert(0);
	
		//prepare for next time
		prePoint=nowPoint;
		nowPoint=nextPoint;
		col=nextCol;  row=nextRow;
		colFloat+=colIncr; rowFloat+=rowIncr;
		nextCol=colFloat+0.5; nextRow=rowFloat+0.5;
		//next point pointer increase
		if(nextCol>col)
			nextPoint++;
		else if(nextCol<col)
			nextPoint--;
		if(nextRow>row)
			nextPoint+=widthStep;
		else if(nextRow<row)
			nextPoint-=widthStep;
	}

	//whether find the edge point?
	if(scanState==scanDone){
		edge_rel_pos(edgePoint, &dx, &dy, widthStep, grayTh);
		tm.transform(edgeCol+dx-halfWidth, halfHeight-(edgeRow+dy), px, py);
		//if(px>3 ||py>3){
		//	printf("col=%d, row=%d, dx=%f, dy=%f, px=%lf, py=%lf,  (%d, %d)->(%d, %d)\n",
			//	edgeCol, edgeRow, dx, dy, px, py, x1,y1, x2, y2 );
		//}
		return true;
	}
	else
		return false;
}

void EdgeRecognizer::scan_edge_in_line_selector(const LineSelector& lineSltr, int stainSize)
{
	int x1=lineSltr.x1(), x2=lineSltr.x2(), y1=lineSltr.y1(), y2=lineSltr.y2();	
	//get scan vector
	Vector2 v12(x2-x1, y2-y1);
	double lv12= vector_lenght(v12);
	double sltrWidth=lineSltr.get_width();
	if(stainSize>sltrWidth/4)
		stainSize=sltrWidth/4;
	assert(sltrWidth>5);
	Vector2 scanVct;
	if(lineSltr.get_direction()==LineSelector::LEFT)
		scanVct.set(v12.y()/lv12*sltrWidth,  -v12.x()/lv12*sltrWidth);
	else
		scanVct.set(-v12.y()/lv12*sltrWidth,  v12.x()/lv12*sltrWidth);

	//clear formerly point
	points.clear();

	//scan edge points according to line selector
	bool sucess;
	double px, py;
	float col, row, colIncr, rowIncr, colLen, rowLen ;
	float svx=scanVct.x()/2, svy=scanVct.y()/2;
	if(abs(x1-x2)>abs(y1-y2)){
		colIncr=1.0*(x2-x1)/abs(x2-x1);
		rowIncr=1.0*(y2-y1)/abs(x2-x1);
	}
	else{
		colIncr=1.0*(x2-x1)/abs(y2-y1);
		rowIncr=1.0*(y2-y1)/abs(y2-y1);
	}
	colLen=fabs(x1-x2); 
	rowLen=fabs(y1-y2);
	for(col=x1, row=y1;fabs(col-x1)<colLen ||fabs(row-y1)<rowLen; col+=colIncr, row+=rowIncr){
		PointSelector pointSltr(col-svx, row-svy, col+svx, row+svy);
		if(stainSize>0)
			sucess=scan_no_stain_edge_in_line(pointSltr, px,  py, stainSize);
		else
			sucess=scan_edge_in_line(pointSltr, px,  py);
		if(sucess)
			points.add(px, py);
	}
}

void EdgeRecognizer::scan_edge_in_circle_selector(const CircleSelector& circleSltr)
{
	ArcSelector::Direction arcSltrDir;
	if(circleSltr.get_direction()==CircleSelector::OUTSIDE)
		arcSltrDir=ArcSelector::OUTSIDE;
	else
		arcSltrDir=ArcSelector::INSIDE;
	ArcSelector arcStlr(circleSltr.cx(), circleSltr.cy(), circleSltr.r(), 0.0, 2*PI, circleSltr.get_width(), arcSltrDir);
	scan_edge_in_arc_selector(arcStlr, true);
}

void EdgeRecognizer::scan_edge_in_arc_selector(const ArcSelector& arcSltr, bool isCircle)
{
	//get arc selector information
	double cx=arcSltr.cx(), cy=arcSltr.cy(), r=arcSltr.r();
	double start=arcSltr.start(), end=arcSltr.end();
	double angleRange=in_2pi(end-start);
	if(isCircle){
		start=0;
		end=2*PI;
		angleRange=2*PI;
	}
	
	//printf("(%f, %f), r=%f, start=%f, end=%f\n", cx, cy, r, start, end);
	double halfWidth=arcSltr.get_width()/2.0;
	double theta, angle, angleIncr=2.0/r;
	
	//printf("angle range =%f\n", angleRange);
	double scanVctx, scanVcty; //scan vector
	double posx, posy;//scan middle point
	double directionSign;
	
	if(arcSltr.get_direction()==ArcSelector::OUTSIDE){
		//printf("out side\n");
		directionSign=1;
	}
	else{
		//printf("in side\n");
		directionSign=-1;
	}

	//clear formerly point
	points.clear();

	//scan edge points according to arc selector
	bool sucess;
	double px, py;
	for(angle=0; angle<angleRange; angle+=angleIncr){
		theta=start+angle;
		scanVctx=directionSign*halfWidth*cos(theta);
		scanVcty=directionSign*halfWidth*sin(theta);
		posx=cx+r*cos(theta);
		posy=cy+r*sin(theta);
		
		PointSelector pointSltr(posx-scanVctx, posy-scanVcty, posx+scanVctx, posy+scanVcty);
		sucess=scan_edge_in_line(pointSltr, px,  py);
		if(sucess &&(fabs(px)>10 ||fabs(py)>10))
			printf("in line (%d, %d)->(%d, %d), x=%f, y=%f\n", pointSltr.x1(), pointSltr.y1(), pointSltr.x2(), pointSltr.y2(), px, py);
		if(sucess)
			points.add(px, py);
		//else 
			//printf("line scan fail\n");
	}
}

/*
MarkRecognizer::MarkRecognizer():smooth(NULL){
	
}

Point MarkRecognizer::recognize(const IplImage * image, MarkType markType, double param1, double param2)
{
	//must be gray image
	assert(image->depth==IPL_DEPTH_8U && image->nChannels==1);
	
	//allocate smooth if image size changed
	if(smooth==NULL)
		smooth=cvCreateImage(cvSize(image->width, image->height),IPL_DEPTH_8U, 1);
	else if(image->width!=smooth->width || image->height!=smooth->height){
		cvReleaseImage(&smooth);
		smooth=cvCreateImage(cvSize(image->width, image->height),IPL_DEPTH_8U, 1);
	}
	
	//get and smooth the image
	cvSmooth(image, smooth, CV_MEDIAN, 3, 0, 0, 0);
	
	
}*/


GeomElement GeomRelationship::figure_up(const IMC & imc, const GeomElement & g1)
{
	IMC::CmdType cmdType = imc.cmd_type();
	assert(g1.type()==GeomElement::LINE);
	bool valid=g1.valid();
	if(cmdType==IMC::MID_POINT){
		Point point;
		if(valid)
			point=mid_point(g1.get_line());
		return GeomElement(point, valid);
	}
	else if(cmdType==IMC::PERPENDICULAR_LINE){
		Line line;
		if(valid)
			line=perpendicular_line(g1.get_line());
		return GeomElement(line, valid);
	}
	else if(cmdType==IMC::EXTEND_LINE){
		Line line;
		if(valid){
			const Line& l=g1.get_line();
			double x1=l.x1(), x2=l.x2(), y1=l.y1(), y2=l.y2();
			double d1=two_points_distance(imc.sel_pos1(), l.p1());
			double d2=two_points_distance(imc.sel_pos1(), l.p2());
			if(d1<d2)
				line=Line(x1+x1-x2, y1+y1-y2, x2, y2);
			else
				line=Line(x1, y1, x2+x2-x1, y2+y2-y1);
		}
		return GeomElement(line, valid);
	}
	else
		assert(0);
}

GeomElement GeomRelationship::figure_up(const IMC & imc, const GeomElement & g1, const GeomElement & g2)
{
	bool valid=g1.valid()&&g2.valid();
	IMC::CmdType cmdType = imc.cmd_type();
	
	if(cmdType==IMC::INTERSECTION_POINT){
		Point point;
		assert(g1.type()==GeomElement::LINE && g2.type()==GeomElement::LINE);
		if(valid){
			Angle angle=two_lines_angle(g1.get_line(), g2.get_line());
			if(angle.degree()>0.1)
	 			point=two_lines_intersection(g1.get_line(), g2.get_line());
			else
				valid=false;
		}
		return GeomElement(point, valid);
	}
	else if(cmdType==IMC::TWO_POINT_LINE){
		Line line;
		assert(g1.type()==GeomElement::POINT && g2.type()==GeomElement::POINT);
		if(valid)
			line=Line(g1.get_point(), g2.get_point());
		return GeomElement(line, valid);
	}
	else if(cmdType==IMC::MID_LINE){
		Line line;
		assert(g1.type()==GeomElement::LINE&& g2.type()==GeomElement::LINE);
		if(valid){
			Line l1=g1.get_line();
			Line l2=g2.get_line();
			Angle angle=two_lines_angle(l1, l2);
			if(angle.degree()>0.1){
				Point ip=two_lines_intersection(l1, l2);
				if((l1.x2()-l1.x1())*(imc.sel_pos1().x()-ip.x())+(l1.y2()-l1.y1())*(imc.sel_pos1().y()-ip.y())<0)
					l1=Line(l1.p2(), l1.p1());		
				if((l2.x2()-l2.x1())*(imc.sel_pos2().x()-ip.x())+(l2.y2()-l2.y1())*(imc.sel_pos2().y()-ip.y())<0)
					l2=Line(l2.p2(), l2.p1());
			}
			else{
					if((l1.x2()-l1.x1())*(l2.x2()-l2.x1())+(l1.y2()-l1.y1())*(l2.y2()-l2.y1())<0)
						l1=Line(l1.p2(), l1.p1());
			}
			line=mid_line(l1, l2);
		}
		return GeomElement(line, valid);
	}
	else if(cmdType==IMC::TWO_POINT_DISTANCE){
		Distance distance;
		assert(g1.type()==GeomElement::POINT && g2.type()==GeomElement::POINT);
		if(valid){
			double dis=two_points_distance(g1.get_point(), g2.get_point());
			distance= Distance(dis);
		}
		return GeomElement(distance, valid);
	}
	else if(cmdType==IMC::POINT_LINE_DISTANCE){
		Distance distance;
		assert(g1.type()==GeomElement::POINT && g2.type()==GeomElement::LINE);
		if(valid){
			double dis=point_line_distance(g1.get_point(), g2.get_line());
			distance= Distance(dis);
		}
		return GeomElement(distance, valid);	
	}
	else if(cmdType==IMC::TWO_LINE_DISTANCE){
		Distance distance;
		assert(g1.type()==GeomElement::LINE && g2.type()==GeomElement::LINE);
		if(valid){
			distance=two_lines_distance(g1.get_line(), g2.get_line());
		}
		return GeomElement(distance, valid);	
	}
	else if(cmdType==IMC::TWO_LINE_ANGLE){
		Angle angle;
		assert(g1.type()==GeomElement::LINE && g2.type()==GeomElement::LINE);
		if(valid){
			angle=two_lines_angle(g1.get_line(), g2.get_line());
			if(angle.degree()>0.1){
				const Point& sp1=imc.sel_pos1();
				const Point& sp2=imc.sel_pos2();
				Point np1=nearest_point_in_line(sp1, g1.get_line());
				Point np2=nearest_point_in_line(sp2, g2.get_line());
				Point ip= two_lines_intersection( g1.get_line(),  g2.get_line());
				Vector2 v1(np1.x()-ip.x(), np1.y()-ip.y());
				Vector2 v2(np2.x()-ip.x(), np2.y()-ip.y());
				angle = Angle(two_vectors_angle(v1, v2));
			}
			else
				valid=false;
		}
		return GeomElement(angle, valid);	
	}
	else if(cmdType==IMC::TWO_CIRCLE_DISTANCE){
		Distance dis;
		//assert((g1.type()==GeomElement::CIRCLE ||g1.type==GeomElement::ARC) && 
		//	(g2.type()==GeomElement::CIRCLE ||g2.type()==GeomElement::ARC));
		Point c1, c2;
		double r1, r2;
		if(g1.type()==GeomElement::CIRCLE){
			c1=g1.get_circle().centre();
			r1=g1.get_circle().r();
		}
		else if(g1.type()==GeomElement::ARC){
			c1=g1.get_arc().centre();
			r1=g1.get_arc().r();
		}
		else
			assert(0);
		if(g2.type()==GeomElement::CIRCLE){
			c2=g2.get_circle().centre();
			r2=g2.get_circle().r();
		}
		else if(g2.type()==GeomElement::ARC){
			c2=g2.get_arc().centre();
			r2=g2.get_arc().r();
		}
		else
			assert(0);
		
		if(valid){
			double ccdis=two_points_distance(c1, c2);
			double max=ccdis+r1+r2;
			double min=ccdis-r1-r2;
			if(min<0)
				min=0;
			dis=Distance(ccdis, max, min);
		}
		return  GeomElement(dis, valid);
	}
	else if(cmdType==IMC::CIRCLE_LINE_DISTANCE){
		Distance dis;
		Point c;
		double r;
		if(g1.type()==GeomElement::CIRCLE){
			c=g1.get_circle().centre();
			r=g1.get_circle().r();
		}
		else if(g1.type()==GeomElement::ARC){
			c=g1.get_arc().centre();
			r=g1.get_arc().r();
		}
		else 
			assert(0);
		assert( g2.type()==GeomElement::LINE);
		if(valid){
			double avg=point_line_distance(c, g2.get_line());
			double max=avg+r;
			double min=avg-r;
			if(min<0)
				min=0;
			dis=Distance(avg, max, min);
		}
		return GeomElement(dis, valid);
	}
	else if(cmdType==IMC::CIRCLE_LINE_INTERSETION){
		Point p1, p2;
		assert( g2.type()==GeomElement::LINE);
		if(valid){
			if(g1.type()==GeomElement::CIRCLE)
				valid=circle_line_interserction(g2.get_line(), g1.get_circle(), p1, p2);
			else if(g1.type()==GeomElement::ARC)
				valid=arc_line_interserction(g2.get_line(), g1.get_arc(), p1, p2);
			else 
				assert(0);
			if(valid){
				double dis1=two_points_distance(imc.sel_pos1(), p1);
				double dis2=two_points_distance(imc.sel_pos1(), p2);
				if(dis1>dis2)
					p1=p2;
			}
		}
		return GeomElement(p1, valid);
	}
	else if(cmdType==IMC::TWO_CIRCLE_TANGENT){
		Line tangentLine;
		if(valid){
			Circle c1, c2;
			double dis1, dis2, dis, disMin=10000;
			vector<Line> tangents;
			vector<Line>::iterator lineIt; 
			if(g1.type()==GeomElement::CIRCLE)
				c1=g1.get_circle();
			else if(g1.type()==GeomElement::ARC)
				c1=Circle(g1.get_arc().centre(), g1.get_arc().r());
			else
				assert(0);
			if(g2.type()==GeomElement::CIRCLE)
				c2=g2.get_circle();
			else if(g2.type()==GeomElement::ARC)
				c2=Circle(g2.get_arc().centre(), g2.get_arc().r());
			else
				assert(0);
			valid=two_circles_tangent(c1, c2, tangents);
			if(valid){
				for(lineIt=tangents.begin(); lineIt!=tangents.end(); lineIt++){
					dis1=point_line_distance(imc.sel_pos1(), *lineIt);
					dis2=point_line_distance(imc.sel_pos2(), *lineIt);
					dis=dis1+dis2;
					if(dis<disMin){
						disMin=dis;
						tangentLine=*lineIt;
					}
				}
			}
		}
		return GeomElement(tangentLine, valid);
	}
	else if(cmdType==IMC::TWO_CIRCLE_MID_LINE){
		Line ml;
		if(valid){
			Point c1, c2;
			if(g1.type()==GeomElement::CIRCLE)
				c1=g1.get_circle().centre();
			else if(g1.type()==GeomElement::ARC)
				c1=g1.get_arc().centre();
			else
				assert(0);
			if(g2.type()==GeomElement::CIRCLE)
				c2=g2.get_circle().centre();
			else if(g2.type()==GeomElement::ARC)
				c2=g2.get_arc().centre();
			else
				assert(0);
			ml=perpendicular_line(Line(c1, c2));
		}
		return GeomElement(ml, valid);
	}
	else if(cmdType==IMC::TWO_CIRCLE_INTERSETION){
		Point intersection;
		if(valid){
			Circle c1, c2;
			Point p1, p2;
			double dis1, dis2;
			if(g1.type()==GeomElement::CIRCLE)
				c1=g1.get_circle();
			else if(g1.type()==GeomElement::ARC)
				c1=Circle(g1.get_arc().centre(), g1.get_arc().r());
			else
				assert(0);
			if(g2.type()==GeomElement::CIRCLE)
				c2=g2.get_circle();
			else if(g2.type()==GeomElement::ARC)
				c2=Circle(g2.get_arc().centre(), g2.get_arc().r());
			else
				assert(0);
			valid=two_circles_intersection(c1, c2, p1, p2);
			if(valid){
				dis1=two_points_distance(p1, imc.sel_pos1())+two_points_distance(p1, imc.sel_pos2());
				dis2=two_points_distance(p2, imc.sel_pos1())+two_points_distance(p2, imc.sel_pos2());
				if(dis1<dis2)
					intersection=p1;
				else
					intersection=p2;
			}
		}
		return GeomElement(intersection, valid);
	}
	else
		assert(0);
}

void EdgeGeomCombination::add(const PointsArray & pointArray)
{
	
	if(pointArray.valid_num()==1){
		terminals.add(pointArray.first_valid());
	}
	else{
		terminals.add(pointArray.first_valid());
		terminals.add(pointArray.last_valid());
	}
	points.add(pointArray);
}

GeomElement EdgeGeomCombination::figure_up(const IMC& imc)
{
	bool valid;
	if(imc.cmd_type()==IMC::COMB_LINE){
		if(points.valid_num()<2)
			return GeomElement(Line(), false);
		Line line=comb_line(valid);
		return GeomElement(line, valid);
	}
	else if(imc.cmd_type()==IMC::COMB_CIRCLE){
		if(points.valid_num()<3)
			return GeomElement(Circle(), false);
		Circle circle=comb_circle(valid);
		return GeomElement(circle, valid);
	}
	else if(imc.cmd_type()==IMC::COMB_ARC){
		if(points.valid_num()<3)
			return GeomElement(Arc(), false);
		Arc arc=comb_arc(valid);
		return GeomElement(arc, valid);
	}
	else
		assert(0);
}

Line EdgeGeomCombination::comb_line(bool& sucess)
{
	int type;
	Line l;
	points.sample(maxPoints);
	const ValidPoint& first=points.first_valid();
	const ValidPoint& last=points.last_valid();
	double dx=first.x-last.x;
	double dy=first.y-last.y;
	if(fabs(dx)>fabs(dy))
		type=Y_EQU_FX;
	else
		type=X_EQU_FY;
	sucess=line_LSM(points, type, l, 10, 2);
	points.clear();
	terminals.clear();
	return l;
}

Circle EdgeGeomCombination::comb_circle(bool& sucess){
	Circle circle;
	points.sample(maxPoints);
	sucess=circle_LSM(points, circle, 10, 3);
	points.clear();
	terminals.clear();
	return circle;
}

Arc EdgeGeomCombination::comb_arc(bool& sucess){
	//Arc arc;
	int i,j;
	double x1, y1, x2, y2, cx, cy, ang1, ang2, ang;
	double arcStart=0, arcEnd=0,  maxAng=0;
	Circle circle;
	points.sample(maxPoints);
	sucess=circle_LSM(points, circle, 10, 3);
	cx=circle.cx(); cy=circle.cy();
	int termNum= terminals.size();
	for(i=0;i<termNum;i++){
		x1=terminals[i].x;
		y1=terminals[i].y;
		ang1=arctan(x1-cx, y1-cy);
		for(j=i+1;j<termNum;j++){
			x2=terminals[j].x;
			y2=terminals[j].y;
			ang2=arctan(x2-cx, y2-cy);
			ang=in_2pi(ang2-ang1);
			if(ang<PI){
				if(ang>maxAng){
					maxAng=ang;
					arcStart=ang1;
					arcEnd=ang2;
				}
			}
			else{
				ang=2*PI-ang;
				if(ang>maxAng){
					maxAng=ang;
					arcStart=ang2;
					arcEnd=ang1;
				}
			}
		}
	}
	points.clear();
	terminals.clear();
	return Arc(cx, cy, circle.r(), arcStart, arcEnd);
}


HoughTransformer::HoughTransformer(double rho, double theta, double maxRho, double thetaBegin, double thetaEnd, int linesNumMax, Point center)
	:m_rho(rho), m_theta(theta), m_thetaBegin(thetaBegin), linesMax(linesNumMax), m_center(center)
{
	assert(thetaEnd>thetaBegin && thetaEnd-thetaBegin<180);
	assert(rho>0.001 && theta>0.1/180*PI && maxRho>0.001);
	
	numangle= (int)((thetaEnd-thetaBegin) / theta);
	tabSin=(float*)malloc(numangle*sizeof(float));
	tabCos=(float*)malloc(numangle*sizeof(float));
	assert(tabSin&&tabCos);

	numrho=(int)(maxRho/rho);
	numrho2=2*numrho+1;
	
	accum= (int*)malloc(numangle *numrho2*sizeof(long));
	sortAccum= (long*)malloc(numangle *numrho2*sizeof(long));
	assert(accum&& sortAccum);
	float irho = 1 / rho; 
	float ang;
	int n;
	for( ang = thetaBegin, n = 0; n < numangle; ang += theta, n++ ){
		tabSin[n] = (float)(sin(ang) * irho);
		tabCos[n] = (float)(cos(ang) * irho);
	 }
}

void HoughTransformer::get_hough_lines(const vector <Point> & points,vector <HoughLine> & lines, int threshold)const
{
	float x0=m_center.x();
	float y0=m_center.y();
	float dist;
	long total=0;
	int n, r=0;
	memset( accum, 0, sizeof(int) *numangle *numrho2);
	vector<Point>::const_iterator pIt;

	//stage1 : fill the accumulator
	for(pIt=points.begin(); pIt!=points.end(); pIt++){
		for( n = 0; n < numangle; n++ ){
			dist= ((float(pIt->x()))-x0)* tabCos[n] + ((float(pIt->y()))-y0)* tabSin[n];
			//dist is not the actual distanc, becasue have*irho
			if(dist>=0 && dist<numrho){
				r=numrho+long(dist+0.5);
				accum[n *numrho2 + r]++;
			}
			else if(dist<0 && dist>-numrho){
				r=numrho+long(dist-0.5);
				accum[n *numrho2 + r]++;
			}
		}
	}
	
	// stage 2. find local maximums
	int center, top, bottom,left, right;
	int i, j;
    for( i= 2; i< numangle-2;i++ ){
        for( j = 2; j < numrho2-2; j++ )
        {
			center=area_accum(i,j);
			if( center> threshold ){
				top=area_accum(i-1,j);
				bottom=area_accum(i+1, j);
				left=area_accum(i,j-1);
				right=area_accum(i,j+1);
				if(center>top && center>=bottom && center>left && center>=right)
					sortAccum[total++]=i*numrho2+j;
			}
        }
    }

	// stage 3. sort the detected lines by accumulator value
	int maxPoints;
	long maxi;
	int temp;
	int linesNum = MIN(linesMax, total);
	for(i=0;i<linesNum;i++){
		maxPoints=accum[sortAccum[i]];
		maxi=i;
		for(j=i+1;j<total;j++){
			if(accum[sortAccum[j]]>maxPoints){
				maxPoints=accum[sortAccum[j]];
				maxi=j;
			}
		}
		if(maxi!=i){
			temp=sortAccum[i];
			sortAccum[i]=sortAccum[maxi];
			sortAccum[maxi]=temp;
		}
	}

	//stage4: output the hough lines
	lines.clear();
	long index;
	int angleIndex;
	int rhoIndex;
	for(i=0;i<linesNum;i++){
		index=sortAccum[i];
		angleIndex=index/numrho2;
		rhoIndex=index%numrho2;
		HoughLine aline(m_thetaBegin+ angleIndex*m_theta, (rhoIndex-numrho)*m_rho);
		lines.push_back(aline);
	}
}

int HoughTransformer::area_accum(int angleIndex, int rhoIndex) const
{
	assert(angleIndex>0 && angleIndex<numangle-1);
	assert(rhoIndex>0 && rhoIndex<numrho2-1);
	int* center=accum+angleIndex*numrho2+rhoIndex;
	int* up=center-numrho2;
	int* down=center+numrho2;
	int sum=center[0]+center[-1]+center[1]+up[0]+up[-1]+up[1]+down[0]+down[-1]+down[1];
	return sum;
}
HoughTransformer::~HoughTransformer(){
	free(tabSin);
	free(tabCos);
	free(accum);
	free(sortAccum);
}

/*subPixel: get relative position by linear interpolation*/
static void edge_rel_pos(unsigned char *imgPoint, float *x, float *y, int step, int thresh)
{
	unsigned char dpe, dpp; //difference of point and edge and difference fo point and point
	*x=0;
	*y=0;
	if(imgPoint[0]<thresh)
	{
		dpe=thresh-imgPoint[0];
		
		if(imgPoint[-1]>imgPoint[0]+30 ||imgPoint[1]>imgPoint[0]+30)
		{
			if(imgPoint[-1]>thresh)
			{
				dpp=imgPoint[-1]-imgPoint[0];
				*x=-1.0f*dpe/dpp;
			}
			else if(imgPoint[1]>thresh)
			{
				dpp=imgPoint[1]-imgPoint[0];
				*x=1.0f*dpe/dpp;
			}
			else if(imgPoint[-2]>thresh)
			{
				dpp=imgPoint[-2]-imgPoint[0];
				*x=-2.0f*dpe/dpp;
			}
			else if(imgPoint[2]>thresh)
			{
				dpp=imgPoint[2]-imgPoint[0];
				*x=2.0f*dpe/dpp;
			}
			else
				*x=0;
		}
		else
			*x=0;
		
		if(imgPoint[-step]>imgPoint[0]+30 ||imgPoint[step]>imgPoint[0]+30)
		{
			if(imgPoint[-step]>thresh)
			{
				dpp=imgPoint[-step]-imgPoint[0];
				*y=-1.0f*dpe/dpp;
			}
			else if(imgPoint[step]>thresh)
			{
				dpp=imgPoint[step]-imgPoint[0];
				*y=1.0f*dpe/dpp;
			}
			else if(imgPoint[-(step+step)]>thresh)
			{
				dpp=imgPoint[-(step+step)]-imgPoint[0];
				*y=-2.0f*dpe/dpp;
			}
			else if(imgPoint[step+step]>thresh)
			{
				dpp=imgPoint[step+step]-imgPoint[0];
				*y=2.0f*dpe/dpp;
			}
			else
				*y=0;
		}
		else
			*y=0;
	}
	else if(imgPoint[0]>thresh)
	{
		dpe=imgPoint[0]-thresh;
		
		if(imgPoint[0]>imgPoint[-1]+30 ||imgPoint[0]>imgPoint[1]+30)
		{
			if(imgPoint[-1]<thresh)
			{
				dpp=imgPoint[0]-imgPoint[-1];
				*x=-1.0f*dpe/dpp;
			}
			else if(imgPoint[1]<thresh)
			{
				dpp=imgPoint[0]-imgPoint[1];
				*x=1.0f*dpe/dpp;
			}
			else if(imgPoint[-2]<thresh)
			{
				dpp=imgPoint[0]-imgPoint[-2];
				*x=-2.0f*dpe/dpp;
			}
			else if(imgPoint[2]<thresh)
			{
				dpp=imgPoint[0]-imgPoint[2];
				*x=2.0f*dpe/dpp;
			}
			else
				*x=0;
		}
		else
			*x=0;
		if(imgPoint[0]>imgPoint[-step]+30 ||imgPoint[0]>imgPoint[step]+30)
		{
			if(imgPoint[-step]<thresh)
			{
				dpp=imgPoint[0]-imgPoint[-step];
				*y=-1.0f*dpe/dpp;
			}
			else if(imgPoint[step]<thresh)
			{
				dpp=imgPoint[0]-imgPoint[step];
				*y=1.0f*dpe/dpp;
			}
			else if(imgPoint[-(step+step)]<thresh)
			{
				dpp=imgPoint[0]-imgPoint[-(step+step)];
				*y=-2.0f*dpe/dpp;
			}
			else if(imgPoint[step+step]<thresh)
			{
				dpp=imgPoint[0]-imgPoint[step+step];
				*y=2.0f*dpe/dpp;
			}
			else
				*y=0;
		}
		else
			*y=0;
	}
	else
	{
		*x=0;
		*y=0;
	}
}


/*subPixel: get relative position by linear interpolation*/
 void edge_rel_pos2(unsigned char *imgPoint, float *x, float *y, int step, int thresh)
{
	unsigned char dpe=0, dpp=0; //difference of point and edge and difference fo point and point
	int i, posStep=0, negStep=0;
	const int range=4;
	*x=0;
	*y=0;
	if(imgPoint[0]<thresh)
	{
		dpe=thresh-imgPoint[0];
		for(i=1;i<=range;i++){
			if(imgPoint[-i]>thresh){
				dpp=imgPoint[-i]-imgPoint[0];	
				*x=-i*1.0f*dpe/dpp;
				break;
			}
			else if(imgPoint[i]>thresh){
				dpp=imgPoint[i]-imgPoint[0];
				*x=i*1.0f*dpe/dpp;
				break;
			}
		}
		if(i>range)
			*x=0;

		for(i=1, posStep=step, negStep=-step;i<=range; i++, posStep+=step, negStep-=step){
			if(imgPoint[negStep]>thresh){
				dpp=imgPoint[negStep]-imgPoint[0];
				*y=-i*1.0f*dpe/dpp;
				break;
			}
			else if(imgPoint[posStep]>thresh){
				dpp=imgPoint[posStep]-imgPoint[0];
				*y=i*1.0f*dpe/dpp;
				break;
			}
		}
		if(i>range)
			*y=0;
	}
	else if(imgPoint[0]>thresh)
	{
		dpe=imgPoint[0]-thresh;
		
		for(i=1;i<=range;i++){
			if(imgPoint[-i]<thresh){
				dpp=imgPoint[0]-imgPoint[-i];	
				*x=-i*1.0f*dpe/dpp;
				break;
			}
			else if(imgPoint[i]<thresh){
				dpp=imgPoint[0]-imgPoint[i];
				*x=i*1.0f*dpe/dpp;
				break;
			}
		}
		if(i>range)
			*x=0;

		for(i=1, posStep=step, negStep=-step;i<=range; i++, posStep+=step, negStep-=step){
			if(imgPoint[negStep]<thresh){
				dpp=imgPoint[0]-imgPoint[negStep];
				*y=-i*1.0f*dpe/dpp;
				break;
			}
			else if(imgPoint[posStep]<thresh){
				dpp=imgPoint[0]-imgPoint[posStep];
				*y=i*1.0f*dpe/dpp;
				break;
			}
		}
		if(i>range)
			*y=0;		
	}
	else{
		*x=0;
		*y=0;
	}
	if(fabs(*x)>0.1 ||fabs(*y)>0.1)
		printf("dx=%lf, dy=%lf, dpe=%d, dpp=%d\n", *x, *y, dpe, dpp);
}


double in_bound(double data, double min, double max)
{
	if(data<min)
		return min;
	else if(data>max)
		return max;
	else 
		return data;
}

/*figure up the square of distance of point off line,
p1--the fisrt point of line, p2--the 2nd point of line, p-- the independent point*/
static double pld2( const ValidPoint& p1, const ValidPoint&  p2, const ValidPoint&  p)
{
	double vct1x=p2.x-p1.x;
	double vct1y=p2.y-p1.y;
	double vct2x=p.x-p1.x;
	double vct2y=p.y-p1.y;
	/*inner product*/
	double vct12IP=vct1x*vct2x+vct1y*vct2y;
	double vct1IP=vct1x*vct1x+vct1y* vct1y;
	double vct2IP=vct2x*vct2x+vct2y*vct2y;
	return vct2IP-vct12IP*vct12IP/vct1IP;
}

/*remove roughly the apprarently uncorrelated point from line edge*/
bool  line_point_filter(PointsArray& points, float rmDis)
{
	static int recurTimes=0;
	static const int maxTimes=20;
	static const float errorRate=0.25;
	if(recurTimes>maxTimes)
	{
		printf("lineFilter recur too many times, so image noisy is terrible\n");
		recurTimes=0;
		return false;
	}
	recurTimes++;
	int num=points.size();
	
	//separate all line data into 3 parts, select two random points from 1st and 3rd part.
	int n1=num/3*(1.0*rand()/RAND_MAX);
	int n2=num/3*2+num/3*(1.0*rand()/RAND_MAX);
	ValidPoint p1=points[n1];
	ValidPoint p2=points[n2];
	//ValidPoint p1=points[(int)(num*(1.0*rand()/RAND_MAX))];
	//ValidPoint p2=points[(int)(num*(1.0*rand()/RAND_MAX))];
	int badNum=0;
	double maxdd=rmDis*rmDis;
	double dd;
	vector<ValidPoint>::iterator it;
	for(it=points.begin();it!=points.end();it++){
		dd=pld2(p1, p2, *it);
		if(dd<maxdd)
			it->valid=true;
		else{
			it->valid=false;
			badNum++;
		}
	}
	if(badNum>num*errorRate)
		return line_point_filter(points, rmDis);
	recurTimes=0;
	return true;	
}


/*the least square method for straight line*/
bool line_LSM(PointsArray& points, int& type, double& k, double& b, double maxDistance, long minPN)
{
	static int recurTimes=0;
	double sum_x=0;
	double sum_y=0;
	double sum_xy=0;
	double sum_xx=0;
	double sum_yy=0;
	long sum_valid=0;
	long reLsm=0;

	assert(type==Y_EQU_FX ||type==X_EQU_FY);

	recurTimes++;
	if(recurTimes>10){
		printf("Function lineLsm recur too many times\n");
		recurTimes=0;
		return false;
	}
	double x=0,y=0,err=0;
	vector<ValidPoint>::iterator it;
	for(it=points.begin(); it!=points.end(); it++){
		if(it->valid){
			sum_valid++;
			x=it->x;
			y=it->y;
			sum_x+=x;
			sum_y+=y;
			sum_xy+=x*y;
			sum_xx+=x*x;
			sum_yy+=y*y;
		}
	}
	if(sum_valid<minPN){
		printf("\nIn function lineLsm(), only have %ld valid data, too few\n",sum_valid);
		printf("k=%lf, b=%lf \n", k, b);
		recurTimes=0;
		return false;
	}
	maxDistance=fabs(maxDistance);
	if(type==Y_EQU_FX){
		k=(sum_valid*sum_xy-sum_x*sum_y)/(sum_valid*sum_xx-sum_x*sum_x);
		b=(sum_y-k*sum_x)/sum_valid;

		//if not fit to use this type of lsm
		if(k>1.5 || k<-1.5){
			type=X_EQU_FY;
			return line_LSM(points,type,k,b,maxDistance,minPN);
		}

		//check the error  to remove uncorrelated points
		for(it=points.begin();it!=points.end();it++)
		{
			if(it->valid)
			{
				err=it->y-(k*it->x+b);
				if(err>maxDistance|| err<-maxDistance)
				{
					it->valid=false;
					reLsm=1;
				}
			}
		}
	}
	
	else if(type==X_EQU_FY)
	{
		k=(sum_valid*sum_xy-sum_x*sum_y)/(sum_valid*sum_yy-sum_y*sum_y);
		b=(sum_x-k*sum_y)/sum_valid;

		//if not fit to use this type of lsm
		if(k>1.5 || k<-1.5)
		{
			type=Y_EQU_FX;
			//typeChange=1;
			return line_LSM(points,type,k,b,maxDistance,minPN);
		}

		//check error to remove uncorrelated points
		for(it=points.begin();it!=points.end();it++)
		{
			if(it->valid)
			{
				err=it->x-(k*it->y+b);
				if(err>maxDistance|| err<-maxDistance)
				{
					it->valid=false;
					reLsm=1;
				}
			}
		}
	}
	
	//after remove uncorrelated points , lsm again
	if(reLsm){
		return line_LSM(points,type,k,b,maxDistance,minPN);
	}
	else{// remain points are all correlated
		recurTimes=0;
		return true;
	}
}

bool line_LSM(PointsArray& points, int& type, Line& l, double maxDistance, long minPN)
{
	double k, b;
	bool ok;
	ok=line_LSM(points, type, k, b, maxDistance, minPN);
	if(!ok)
		return false;
	vector<ValidPoint>::iterator it;
	double maxX, minX, maxY, minY;
	if(type==Y_EQU_FX){
		for(it=points.begin(), maxX=minX=it->x;it!=points.end();it++){
			if(it->valid){
				if(it->x>maxX)
					maxX=it->x;
				if(it->x<minX)
					minX=it->x;
			}
		}
		l.set_x1(minX);
		l.set_y1(k*minX+b);
		l.set_x2(maxX);
		l.set_y2(k*maxX+b);
	}
	else{
		for(it=points.begin(), maxY=minY=it->y;it!=points.end();it++){
			if(it->valid){
				if(it->y>maxY)
					maxY=it->y;
				if(it->y<minY)
					minY=it->y;
			}
		}
		l.set_y1(minY);
		l.set_x1(k*minY+b);
		l.set_y2(maxY);
		l.set_x2(k*maxY+b);
	}
	return true;
}

void  lsm_order2(const vector<CvPoint2D32f> & xyArry, double& a, double& b, double& c)
{
	long n=xyArry.size();
	assert(n>=3);
	vector<CvPoint2D32f>::const_iterator it;
	double  x, y, xx, xy;
	double sx=0, sx2=0, sx3=0, sx4=0, sy=0, sxy=0, sx2y=0;
	double det, numa, numb, numc;
	for(it=xyArry.begin(); it!=xyArry.end(); it++){
		xx=x=it->x; xy=y=it->y;
		sx+=x; sy+=y;
		xx*=x; xy*=x;
		sx2+=xx; sxy+=xy;
		xx*=x; xy*=x;
		sx3+=xx;  sx2y+=xy;
		xx*=x;
		sx4+=xx;
	}

	det=sx4*sx2*n + sx3*sx2*sx+sx3*sx2*sx-sx2*sx2*sx2-sx3*sx3*n-sx4*sx*sx;
	numa=sx2y*sx2*n + sx3*sx*sy + sxy*sx*sx2 - sy*sx2*sx2 - sxy*sx3*n - sx2y*sx*sx;
	numb=sx4*sxy*n + sx2y*sx*sx2+ sx3*sy*sx2 -sx2*sxy*sx2 - sx2y*sx3*n - sx*sy*sx4;
	numc= sx4*sx2*sy+ sx3*sxy*sx2+ sx3*sx*sx2y - sx2y*sx2*sx2- sxy*sx4*sx- sy*sx3*sx3;

	a=numa/det;
	b=numb/det;
	c=numc/det;
}


 /*remove roughly the apprarently uncorrelated point for line edge*/
 //return the number of remain vaild points
bool circle_point_filter(PointsArray& points, double rmDistance)
{
	static int recurTimes=0;
	static const int maxTimes=25;
	static const float errorRate=0.25;
	
	if(recurTimes>maxTimes){
		printf("circleFilter recur too many times, so image noisy is terrible\n");
		recurTimes=0;
		return false;
	}
	recurTimes++;

	int num=points.size();
	int n1=(int)(num*(1.0*rand()/RAND_MAX));
	int n2=(int)(num*(1.0*rand()/RAND_MAX));
	int n3=(int)(num*(1.0*rand()/RAND_MAX));
	Point p1(points[n1].x, points[n1].y);
	Point p2(points[n2].x, points[n2].y);
	Point p3(points[n3].x, points[n3].y);
	double x0,y0,R;
	
	if(!three_points_circle(p1, p2, p3, x0, y0, R))
		return  circle_point_filter(points, rmDistance);
	
	long badNum=0;
	double maxRR=(R+fabs(rmDistance))*(R+fabs(rmDistance));
	double tmp=R-fabs(rmDistance);
	if(tmp<0)
		tmp=0;
	double minRR=tmp*tmp;
	double RR;

	vector<ValidPoint>::iterator it;
	for(it=points.begin(); it!=points.end(); it++){
		RR=(it->x-x0)*(it->x-x0)+(it->y-y0)*(it->y-y0);
		if(RR<maxRR && RR>minRR)
			it->valid=true;
		else{
			it->valid=false;
			badNum++;
		}
	}
	if(badNum>num*errorRate)
		return circle_point_filter(points, rmDistance);
	recurTimes=0;
	return true;	
}


/*the least square method for circle*/
static bool circle_direct_LSM(PointsArray& points, double& x0, double& y0, double& R, double rmDistance , long minPN)
{
	static int recurTimes=0;
	double sum_z=0;
	double sum_x=0;
	double sum_y=0;
	double sum_xy=0;
	double sum_xx=0;
	double sum_yy=0;
	double sum_xz=0;
	double sum_yz=0;
	int sum_valid=0;
	double x,y,z,xx,yy;
	
	recurTimes++;
	if(recurTimes>10)
	{
		printf("Function circle_LSM recur too many times\n");
		recurTimes=0;
		return false;
	}
	
	vector<ValidPoint>::iterator it;
	for(it=points.begin(); it!=points.end(); it++){
		if(it->valid){
			sum_valid++;
			x=it->x;
			y=it->y;
			sum_x+=x;
			sum_y+=y;
			sum_xy+=x*y;
			xx=x*x;
			sum_xx+=xx;
			yy=y*y;
			sum_yy+=yy;
			z=xx+yy;
			sum_z+=z;
			sum_xz+=x*z;
			sum_yz+=y*z;
		}
	}
	if(sum_valid<minPN){
		printf("In function lineLsm(), only have %d valid data, too few\n",sum_valid);
		recurTimes=0;
		return false;
	}
	//printf("lsm %d\n", sum_valid);
	double Denominator=sum_valid*sum_xx*sum_yy+2.0*sum_x*sum_xy*sum_y-sum_x*sum_x*sum_yy-sum_valid*sum_xy*sum_xy-sum_xx*sum_y*sum_y;
	double x0_numerator=0.5*(sum_valid*sum_xz*sum_yy+sum_xy*sum_y*sum_z+sum_x*sum_y*sum_yz-sum_x*sum_yy*sum_z-sum_valid*sum_xy*sum_yz-sum_xz*sum_y*sum_y);
	double y0_numerator=0.5*(sum_valid*sum_xx*sum_yz+sum_x*sum_xz*sum_y+sum_x*sum_xy*sum_z-sum_x*sum_x*sum_yz-sum_xx*sum_y*sum_z-sum_valid*sum_xy*sum_xz);
	double r_numerator=sum_xx*sum_yy*sum_z+sum_x*sum_xy*sum_yz+sum_xy*sum_xz*sum_y-sum_x*sum_xz*sum_yy-sum_xx*sum_y*sum_yz-sum_xy*sum_xy*sum_z;

	x0=x0_numerator/Denominator;
	y0=y0_numerator/Denominator;
	R=sqrt(x0*x0+y0*y0+r_numerator/Denominator);
	double maxRR=(R+fabs(rmDistance))*(R+fabs(rmDistance));
	double tmp=R-fabs(rmDistance);
	if(tmp<0)
		tmp=0;
	double minRR=tmp*tmp;
	double RR;
	bool reLsm=false;
	for(it=points.begin(); it!=points.end(); it++){
		if(it->valid){
			RR=(it->y-y0)*(it->y-y0)+(it->x-x0)*(it->x-x0);
			if(RR>maxRR || RR<minRR){
				it->valid=false;
				reLsm=true;
			}
		}
	}
	if(reLsm){
		return circle_direct_LSM(points, x0, y0, R, rmDistance, minPN);
	}
	else{
		recurTimes=0;
		return true;
	}
}

 bool circle_LSM(PointsArray& points, double& x0, double& y0, double& R, double rmDistance , long minPN)
{
	vector<ValidPoint>::iterator it;
	double sumx=0,  sumy=0, avgx, avgy;
	long num=0;
	bool res;
	
	//figure the average of x and y
	for(it=points.begin(); it!=points.end();it++){
		if(it->valid){
			num++;
			sumx+=it->x;
			sumy+=it->y;
		}
	}
	if(num==0)
		return false;
	avgx=sumx/num;
	avgy=sumy/num;

	//sub the average of x and y to avaid overflowing in LSM
	for(it=points.begin(); it!=points.end();it++){
		if(it->valid){
			it->x-=avgx;
			it->y-=avgy;
		}
	}
	//implement the LSM
	res=circle_direct_LSM( points,  x0,  y0,  R, rmDistance, minPN);

	//recover the original points
	for(it=points.begin(); it!=points.end();it++){
		if(it->valid){
			it->x+=avgx;
			it->y+=avgy;
		}
	}
	x0+=avgx;
	y0+=avgy;
	return res;
}

bool circle_LSM(PointsArray& points, Circle& circle, double rmDistance , long minPN)
{
	bool sucess;
	double cx, cy, r;
	sucess=circle_LSM(points, cx, cy, r, rmDistance, minPN);
	circle=Circle(cx, cy, r);
	return sucess;
}

bool arc_LSM(PointsArray& points, Arc& arc, double rmDistance , long minPN)
{
	bool sucess;
	double cx, cy, r;
	double start, end, mid;
	sucess=circle_LSM(points, cx, cy, r, rmDistance, minPN);
	if(!sucess)
		return false;
	points.valid_only();
	start=arctan(points.first_valid().x-cx, points.first_valid().y-cy);
	end=arctan(points.last_valid().x-cx, points.last_valid().y-cy);
	int midIndex=points.valid_num()/2;
	mid=arctan(points[midIndex].x-cx, points[midIndex].y-cy);
	if(in_2pi(mid-start)+in_2pi(end-mid)>2*PI){
		double tmp=start;
		start=end;
		end=tmp;
	}
	arc=Arc(cx, cy, r, start, end);
	return sucess;
}

/*some geom function*/

int toInt(double x){
	if(x>=0)
		return int(x+0.5);
	else
		return int(x-0.5);
}

double in_2pi(double angle)
{
	int n=floor(angle/2.0/PI);
	return angle-n*2.0*PI;
}

double inner_product(const Vector2& v1, const Vector2& v2){
	return v1.x()*v2.x()+v1.y()*v2.y();
}

double vector_lenght(const Vector2& vct)
{
	return sqrt(inner_product(vct, vct));
}

double outer_product(const Vector2& v1, const Vector2& v2){
	return v1.x()*v2.y()-v2.x()*v1.y();
}


double two_points_distance(const Point& p1, const Point& p2)
{
	double dx=p1.x()-p2.x();
	double dy=p1.y()-p2.y();
	return sqrt(dx*dx+dy*dy);
}


Point mid_point(const Point& p1, const Point& p2)
{
	return Point((p1.x()+p2.x())/2, (p1.y()+p2.y())/2);
}


Point mid_point(const Line& l)
{
	return Point((l.x1()+l.x2())/2, (l.y1()+l.y2())/2);
}

Line mid_line(const Line& l1, const Line& l2){
	double mx1, my1, mx2, my2, k;
	Line p1p1(l1.p1(), l2.p1());
	Line p2p2(l1.p2(), l2.p2());
	Angle ang1=two_lines_angle(l1, p1p1);
	Angle ang2=two_lines_angle(l2, p1p1);
	if(ang1.radian()>1.0E-6){
		k=sin(ang2.radian())/sin(ang1.radian());
		mx1=(l1.x1()+k*l2.x1())/(1+k);
		my1=(l1.y1()+k*l2.y1())/(1+k);
	}
	else{
		mx1=l2.x1();
		my1=l2.y1();
	}
	ang1=two_lines_angle(l1, p2p2);
	ang2=two_lines_angle(l2, p2p2);
	if(ang1.radian()>1.0E-6){
		k=sin(ang2.radian())/sin(ang1.radian());
		mx2=(l1.x2()+k*l2.x2())/(1+k);
		my2=(l1.y2()+k*l2.y2())/(1+k);
	}
	else{
		mx2=l2.x2();
		my2=l2.y2();
	}
	return Line(mx1, my1, mx2, my2);
}

Line perpendicular_line(const Line& line){
	double x1=line.x1(), x2=line.x2(), y1=line.y1(), y2=line.y2();
	double mx=(x1+x2)/2.0;
	double my=(y1+y2)/2.0;
	Vector2 v12(x2-x1, y2-y1);
	Vector2 vp=vector_rotate(v12, 0.5*PI);
	return Line(mx+vp.x()/2.0, my+vp.y()/2.0, mx-vp.x()/2.0, my-vp.y()/2.0);
}

double point_line_distance(const  Point& p, const Line& l)
{
	double vct1x=l.x2()-l.x1();
	double vct1y=l.y2()-l.y1();
	double vct2x=p.x()-l.x1();
	double vct2y=p.y()-l.y1();
	/*inner product*/
	double vct12IP=vct1x*vct2x+vct1y*vct2y;
	double vct1IP=vct1x*vct1x+vct1y* vct1y;
	double vct2IP=vct2x*vct2x+vct2y*vct2y;
	double dis=sqrt(vct2IP-vct12IP*vct12IP/vct1IP);
	return dis;
}

Distance two_lines_distance(const Line& l1, const Line& l2)
{
	int i;
	double averge=0, max, min;
	double d[4];
	Line ml=mid_line(l1,l2);
	max=min=d[0]=point_line_distance(l1.p1(), ml);
	d[1]=point_line_distance(l1.p2(), ml);
	d[2]=point_line_distance(l2.p1(), ml);
	d[3]=point_line_distance(l2.p2(), ml);
	for(i=0;i<4;i++){
		averge+=d[i];
		if(d[i]<min)
			min=d[i];
		if(d[i]>max)
			max=d[i];
	}
	averge/=4;
	return Distance(2*averge, 2*max, 2*min);
}

double two_vectors_angle(const Vector2& v1, const Vector2 v2){
	return acos(inner_product(v1, v2)/vector_lenght(v1)/vector_lenght(v2));
}

 Angle two_lines_angle(const Line& l1, const Line& l2)
{
	double vct1x=l1.x1()-l1.x2();
	double vct1y=l1.y1()-l1.y2();
	double vct2x=l2.x1()-l2.x2();
	double vct2y=l2.y1()-l2.y2();
	double rad=acos(fabs(vct1x*vct2x+vct1y*vct2y)/(sqrt(vct1x*vct1x+vct1y*vct1y)*sqrt(vct2x*vct2x+vct2y*vct2y)));
	Angle angle(rad);
	return angle;
}

void  two_lines_intersection(const Line& l1, const Line& l2, double& x, double& y)
{

	//assert two lines are not parallel
	Angle angle=two_lines_angle(l1, l2);
	assert(angle.degree()>0.1);
	
	//transform to ax+by=c 
	double a1=l1.y2()-l1.y1();
	double b1=l1.x1()-l1.x2();
	double c1=l1.x1()*(l1.y2()-l1.y1())+l1.y1()*(l1.x1()-l1.x2());

	double a2=l2.y2()-l2.y1();
	double b2=l2.x1()-l2.x2();
	double c2=l2.x1()*(l2.y2()-l2.y1())+ l2.y1()*(l2.x1()-l2.x2());

	//figure out intersection point by determinant
	double detAB=a1*b2-a2*b1;
	double detAC=a1*c2-a2*c1;
	double detCB=c1*b2-c2*b1;
	 x=detCB/detAB;
	 y=detAC/detAB;

}


const Point two_lines_intersection(const Line& l1, const Line& l2)
{
	double x, y;
	two_lines_intersection(l1, l2, x, y);
	return Point(x,y);
}

 bool three_points_circle(const Point& p1, const Point& p2, const Point& p3, double& cx, double& cy, double& r)
{
	if(p1==p2 || p1==p3 ||p2==p3){ //some points
		cx=p1.x();
		cy=p1.y();
		r=0;
		return false;
	}
	double x1, x2, x3, y1, y2, y3, z1, z2, z3;
	double x12, x13, y12, y13, z12, z13;
	double detxy, detxz, detzy, D, E;
	double dx, dy;
	//----(x-x0)^2+(y-y0)^2=R^2
	//----x^2+y^2-2x0x-2y0y+x0^2+y0^2-R^2=0
	//----x^2+y^2+Dx+Ey+F=0
	//----Dx+Ey+F=-(x^2+y^2)=z
	//----Dx1+Ey1+F=z1
	//----Dx2+Ey2+F=z2
	//----Dx3+Ey3+F=z3
	//----D(x1-x2)+E(y1-y2)=z1-z2
	//----D(x1-x3)+E(y1-y3)=z1-z3
	x1=p1.x(); y1=p1.y();
	x2=p2.x(); y2=p2.y();
	x3=p3.x(); y3=p3.y();
	z1=-(x1*x1+y1*y1);
	z2=-(x2*x2+y2*y2);
	z3=-(x3*x3+y3*y3);
	x12=x1-x2;
	y12=y1-y2;
	z12=z1-z2;
	x13=x1-x3;
	y13=y1-y3;
	z13=z1-z3;
	detxy=x12*y13-x13*y12;
	detxz=x12*z13-x13*z12;
	detzy=z12*y13-z13*y12;
	D=detzy/detxy;
	E=detxz/detxy;
	cx=-D/2.0;
	cy=-E/2.0;
	dx=x1-cx;
	dy=y1-cy;
	r=sqrt(dx*dx+dy*dy);
	return true;
}


//return rad value in [0,2PI)
double arctan(double x, double y)
{
		if(x<0.0000001 && x>-0.0000001)
		{
			if(y>0)
				return 0.5*PI;
			else
				return 1.5*PI;
		}
		else if(x>=0.0000001)
			return in_2pi(atan(y/x));
		else
			return in_2pi(atan(y/x)+PI);
}

bool in_angle_range(double angle, double start, double end){
	angle=in_2pi( angle);
	start=in_2pi(start);
	end=in_2pi(end);
	if(end>start){
		return angle>=start && angle<=end;
	}
	else{
		return !(angle>end && angle<start);
	}
}

Vector2 vector_rotate(const Vector2& vct, double angle)
{
	double x=vct.x();
	double y=vct.y();
	double x1=cos(angle)*x-sin(angle)*y;
	double y1=sin(angle)*x+cos(angle)*y;
	return Vector2(x1, y1);
}

Vector2 vector_zoom(const Vector2& vct, double times)
{
	return Vector2(vct.x()*times, vct.y()*times);
}

Point nearest_point_in_line(const Point& p, const Line& l)
{
	double vct1x=p.x()-l.x1();
	double vct1y=p.y()-l.y1();
	double vct2x=l.x2()-l.x1();
	double vct2y=l.y2()-l.y1();
	double IP12=vct1x*vct2x+vct1y*vct2y;
	double IP22=vct2x*vct2x+vct2y*vct2y;
	double ratio=IP12/IP22;
	double x=l.x1()+vct2x*ratio;
	double y=l.y1()+vct2y*ratio;
	return Point(x, y);
}

bool circle_line_interserction(const Line& l, const Circle& c, Point& p1, Point& p2){
	double r=c.r();
	Point center=c.centre();
	Point np= nearest_point_in_line(center, l);
	double dis= two_points_distance(center, np);
	double diff=r-dis;
	if(diff<-0.003)
		return false;
	else if(diff<0.003){
		p1=p2=np;
		return true;
	}
	else{
		double halfBowString=sqrt(r*r-dis*dis);
		double len=l.length();
		double dx=(l.x2()-l.x1())/len*halfBowString;
		double dy=(l.y2()-l.y1())/len*halfBowString;
		p1= Point(np.x()+dx, np.y()+dy);
		p2= Point(np.x()-dx, np.y()-dy);
		return true;
	}
}

bool arc_line_interserction(const Line& l, const Arc& arc, Point& p1, Point& p2){
	Circle c(arc.centre(), arc.r());
	return circle_line_interserction(l, c, p1, p2);
}

bool point_circle_tangent(const Point& p, const Circle& c, Line& l1, Line& l2)
{
	Point center=c.centre();
	double r=c.r();
	double dis=two_points_distance(p, center);
	double diff= dis-r;
	if(diff<-0.003){
		return false;
	}
	else if(diff<0.003){
		Vector2 vpc(center.x()-p.x(), center.y()-p.y());
		Vector2 v=vector_rotate(vpc, PI/2);
		l1=l2=Line(p.x()+v.x(), p.y()+v.y(), p.x()-v.x(), p.y()-v.y());
		return true;
	}
	else{
		double theta=acos(r/dis);
		Vector2 vcp((p.x()-center.x())/dis*r, (p.y()-center.y())/dis*r);
		Vector2 v1=vector_rotate(vcp, theta);
		Vector2 v2=vector_rotate(vcp, -theta);
		l1=Line(p.x(), p.y(), center.x()+v1.x(), center.y()+v1.y());
		l2=Line(p.x(), p.y(), center.x()+v2.x(), center.y()+v2.y());
		return true;
	}
}

bool two_circles_tangent(const Circle& c1, const Circle& c2, vector<Line>& tangents)
{
	double tempR, r1=c1.r(), r2=c2.r();
	Point cen1=c1.centre(), cen2=c2.centre(), tempCen;
	//make sure r1>r2
	if(r1<r2){
		tempR=r1;
		r1=r2;
		r2=tempR;
		tempCen=cen1;
		cen1=cen2;
		cen2=tempCen;
	}
	tangents.clear();
	double dis=two_points_distance(cen1, cen2);
	
	if(dis-(r1-r2)<-0.003){
		return false;
	}
	else if(dis-(r1-r2)<0.003){
		Vector2 c12(cen2.x()-cen1.x(), cen2.y()-cen1.y());
		Vector2 v12=vector_zoom(c12, r1/dis);
		Point tanPoint(cen1.x()+v12.x(), cen1.y()+v12.y());
		Vector2 vr=vector_rotate(v12, PI/2);
		Line l(tanPoint.x()+vr.x(), tanPoint.y()+vr.y(), tanPoint.x()-vr.x(), tanPoint.y()-vr.y());
		tangents.push_back(l);
		return true;
	}
	else{
		double theta=acos((r1-r2)/dis);
		Vector2 c12(cen2.x()-cen1.x(), cen2.y()-cen1.y());
		Vector2 vr = vector_rotate(c12, theta);
		Vector2 vr1=vector_zoom(vr, r1/dis);
		Vector2 vr2=vector_zoom(vr, r2/dis);
		tangents.push_back(Line(cen1.x()+vr1.x(), cen1.y()+vr1.y(), cen2.x()+vr2.x(), cen2.y()+vr2.y()));
		vr= vector_rotate(c12, -theta);
		vr1=vector_zoom(vr, r1/dis);
		vr2=vector_zoom(vr, r2/dis);
		tangents.push_back(Line(cen1.x()+vr1.x(), cen1.y()+vr1.y(), cen2.x()+vr2.x(), cen2.y()+vr2.y()));
		double diff=dis-r1+r2;
		if(diff>=-0.003 && diff<=0.003){
			Vector2 v12=vector_zoom(c12, r1/dis);
			Point tanPoint(cen1.x()+v12.x(), cen1.y()+v12.y());
			Vector2 vr=vector_rotate(v12, PI/2);
			Line l(tanPoint.x()+vr.x(), tanPoint.y()+vr.y(), tanPoint.x()-vr.x(), tanPoint.y()-vr.y());
			tangents.push_back(l);
		}
		else if(diff>0.003){
			theta=acos((r1+r2)/dis);
			Vector2 c21=vector_zoom(c12, -1);
			vr1=vector_zoom(vector_rotate(c12, theta), r1/dis);
			vr2=vector_zoom(vector_rotate(c21, theta), r2/dis);
			tangents.push_back(Line(cen1.x()+vr1.x(), cen1.y()+vr1.y(), cen2.x()+vr2.x(), cen2.y()+vr2.y()));
			vr1=vector_zoom(vector_rotate(c12, -theta), r1/dis);
			vr2=vector_zoom(vector_rotate(c21, -theta), r2/dis);
			tangents.push_back(Line(cen1.x()+vr1.x(), cen1.y()+vr1.y(), cen2.x()+vr2.x(), cen2.y()+vr2.y()));
		}
		return true;
	}
}

bool two_circles_intersection(const Circle& c1, const Circle& c2, Point& p1, Point& p2)
{
	double r1=c1.r(), r2=c2.r(), tempR;
	Point cen1=c1.centre(), cen2=c2.centre(), tempCen;
	if(r1<r2){
		tempR=r1;
		r1=r2;
		r2=tempR;
		tempCen=cen1;
		cen1=cen2;
		cen2=tempCen;
	}
	Vector2 c12(cen2.x()-cen1.x(), cen2.y()-cen1.y());
	double dis=two_points_distance(cen1, cen2);
	if(dis<r1-r2-0.003){
		return false;
	}
	else if(dis<r1-r2+0.003){
		Vector2 v12=vector_zoom(c12, r1/dis);
		p1=p2=Point(cen1.x()+v12.x(), cen1.y()+v12.y());
	}
	else if(dis<r1+r2-0.003){
		double theta=acos((dis*dis+r1*r1-r2*r2)/(2*dis*r1));
		Vector2 v12=vector_zoom(c12, r1/dis);
		Vector2 vr=vector_rotate(v12, theta);
		p1=Point(cen1.x()+vr.x(), cen1.y()+vr.y());
		vr=vector_rotate(v12, -theta);
		p2=Point(cen1.x()+vr.x(), cen1.y()+vr.y());
	}
	else if(dis<r1+r2+0.003){
		Vector2 v12=vector_zoom(c12, r1/dis);
		p1=p2=Point(cen1.x()+v12.x(), cen1.y()+v12.y());
	}
	else{
		return false;
	}
	return true;
}

int vertical_modle_line_position(IplImage* img, int step, float theta, int begin, int end, double& diffVal, double* diffs)
{
	int pos;
	int i, j;
//	static int width=0;
	static int height=0;
	static unsigned char** rowPtrs=NULL;
	//static double* diffs=NULL;
	float tanTheta=tan(theta);
	assert(end>begin);
	assert(begin-fabs(tanTheta)*img->height/2-step>1);
	assert(end+fabs(tanTheta)*img->height/2+step < img->width-2);
	
	/*
	if(width!=img->width){
		width=img->width;
		if(diffs)
			free(diffs);
		diffs=(double*)malloc(width*sizeof(double));
	}*/
	
	if(height!=img->height){
		height=img->height;
		if(rowPtrs)
			free(rowPtrs);
		rowPtrs=(unsigned char**)malloc(height*sizeof(unsigned char*));
		assert(rowPtrs);
	}
	int halfHeight=height/2;
	unsigned char* imageData=(unsigned char*)img->imageData;
	int widthStep = img->widthStep;
	float offset;
	double diff=0, maxDiff;
	int maxPos;
	for(i=0;i<height;i++){
		offset=(i-halfHeight)*tanTheta;
		rowPtrs[i]=imageData+i*widthStep+int(begin+offset+0.5);
		for(j=1;j<step;j++)
			diff+=rowPtrs[i][j]-rowPtrs[i][-j];
	}
	maxDiff=fabs(diff);
	maxPos=begin;
	if(diffs)
		diffs[begin]=diff;
	for(pos=begin+1;pos<=end;pos++){
		for(i=0;i<height;i++){
			diff+=rowPtrs[i][step]-rowPtrs[i][1]-(rowPtrs[i][0]-rowPtrs[i][-(step-1)]);
			rowPtrs[i]++;
		}
		if(diffs)
			diffs[pos]=diff;
		if(fabs(diff)>maxDiff){
			maxDiff=fabs(diff);
			maxPos=pos;
		}
	}
	//printf("maxDiff=%lf, pos=%d\n", maxDiff, maxPos);
	diffVal=maxDiff;
	return maxPos;
}



int  horizontal_modle_line_position(IplImage* img, int step, float theta, int begin, int end, double& diffVal, double* diffs)
{
	int pos;
	int i, j;
	static int width=0;
//	static int height=0;
	static unsigned char** ptrs=NULL;
	//static double* diffs=NULL;
	float tanTheta=tan(theta);
	assert(end>begin);
	assert(begin-fabs(tanTheta)*img->width/2-step>1);
	assert(end+fabs(tanTheta)*img->width/2+step < img->height-2);
/*
	if(height!=img->height){
		height=img->height;
		if(diffs)
			free(diffs);
		diffs=(double*)malloc(height*sizeof(double));
	}
	*/
	if(width!=img->width){
		width=img->width;
		if(ptrs)
			free(ptrs);
		ptrs=(unsigned char**)malloc(width*sizeof(unsigned char*));
		assert(ptrs);
	}
	int halfWidth=width/2;
	unsigned char* imageData=(unsigned char*)img->imageData;
	int widthStep = img->widthStep;
	float offset;
	double diff=0, maxDiff;
	int maxPos;
	for(i=0;i<width;i++){
		offset=-(i-halfWidth)*tanTheta;
		ptrs[i]=imageData+widthStep*(int(begin+offset+0.5))+i;
		for(j=1;j<step;j++)
			diff+=ptrs[i][j*widthStep]-ptrs[i][-j*widthStep];
	}
	maxDiff=fabs(diff);
	maxPos=begin;
	if(diffs)
		diffs[begin]=diff;
	for(pos=begin+1;pos<=end;pos++){
		for(i=0;i<width;i++){
			diff+=ptrs[i][step*widthStep]-ptrs[i][widthStep]
					-(ptrs[i][0]-ptrs[i][-(step-1)*widthStep]);
			ptrs[i]+=widthStep;
		}
		if(diffs)
			diffs[pos]=diff;
		if(fabs(diff)>maxDiff){
			maxDiff=fabs(diff);
			maxPos=pos;
		}
	}
	diffVal=maxDiff;
	//printf("maxDiff=%lf, pos=%d\n", maxDiff, maxPos);
	return maxPos;
}

ModelLinePosition::ModelLinePosition(LineType lineType, int step)
	:type(lineType), m_step(step), len(0), scanW(0), pointPtrs(NULL), diffVal(NULL){
}

ModelLinePosition::ModelLinePosition(const ModelLinePosition & other){
	pointPtrs=NULL;
	diffVal=NULL;
	*this=other;
}

void ModelLinePosition::operator=(const ModelLinePosition & other){
	int i;
	type=other.type;
	m_step=other.m_step;
	head=other.head;
	tail=other.tail;
	max=other.max;
	min=other.min;
	len=other.len;
	scanW=other.scanW;
	if(pointPtrs)
		free(pointPtrs);
	pointPtrs=(unsigned char**)malloc(len*sizeof(unsigned char*));
	assert(pointPtrs);
	for(i=0;i<len;i++)
		pointPtrs[i]=other.pointPtrs[i];
	if(diffVal)
		free(diffVal);
	diffVal=(double*)malloc(scanW*sizeof(double));
	assert(diffVal);
	for(i=0;i<scanW;i++)
		diffVal[i]=other.diffVal[i];
}



ModelLinePosition::~ModelLinePosition(){
	if(pointPtrs)
		free(pointPtrs);
	if(diffVal)
		free(diffVal);
}

void ModelLinePosition::set_range(int lineHead, int lineTail, int minPos, int maxPos){
	assert(lineTail>lineHead);
	assert(maxPos>minPos);

	//printf("head=%d, tail=%d, min=%d, max=%d\n", head, tail, min, max);
	head=lineHead;
	tail=lineTail;
	min=minPos;
	max=maxPos;
	//printf("head=%d, tail=%d, min=%d, max=%d\n", head, tail, min, max);
	int newLen=tail-head+1;
	if(newLen!=len){
		len=newLen;
		if(pointPtrs)
			free(pointPtrs);
		pointPtrs=(unsigned char**)malloc(len*sizeof(unsigned char*));
		assert(pointPtrs);
	//	printf("allocate pointptrs\n");
	}
	int newScanW=max-min+1;
	if(newScanW!=scanW){
		scanW=newScanW;
		if(diffVal)
			free(diffVal);
		diffVal=(double*)malloc(scanW*sizeof(double));
	//	printf("allocate diffVal\n");
	}
}

double ModelLinePosition::figure_positon(const IplImage * img, double theta, CvPoint2D32f & pos){
	if(type==vertical)
		return vertical_positon(img, theta, pos);
	else
		return horizontal_positon( img,  theta, pos);
}

double ModelLinePosition::horizontal_positon(const IplImage * img, double theta, CvPoint2D32f & pos){
	int i,j;
	double tanTheta=-tan(theta);
	int offset;
	float point5=0.5;
	int widthStep = img->widthStep;
	unsigned char*headData=(unsigned char*)(img->imageData)+min*widthStep+head;
	double diff=0, maxDiff;
	int maxIndex;
	int add1=m_step*widthStep;
	int add2=-(m_step-1)*widthStep;
	float posx=head, posy=0;

	//printf("theta=%lf, start\n", theta);
	//printf("head=%d, tail=%d, min=%d, max=%d, len=%d, scanW=%d, widthStep=%d\n", head, tail, min, max, len, scanW, widthStep);

	if(tanTheta<0)
		point5=-0.5;
	
	for(i=0;i<len;i++){
		offset=i*tanTheta+point5;
		pointPtrs[i]=headData+offset*widthStep+i;
		for(j=1;j<m_step;j++)
			diff+=pointPtrs[i][j*widthStep]-pointPtrs[i][-j*widthStep];
	}
	maxDiff=diffVal[0]=fabs(diff);
	maxIndex=0;

	//printf("model line figure init ok\n");

	for(i=1; i<scanW; i++){
		for(j=0;j<len;j++){
			diff+=pointPtrs[j][add1]-pointPtrs[j][widthStep]-(pointPtrs[j][0]-pointPtrs[j][add2]);
			pointPtrs[j]+=widthStep;
		}
		diffVal[i]=fabs(diff);
		if(diffVal[i]>maxDiff){
			maxDiff=diffVal[i];
			maxIndex=i;
		}
	}	
	//printf("model line figure  ok\n");

	if(maxIndex!=0 && maxIndex!=scanW-1)
	{
		double a,b, c;
		posDiffArry.clear();
		posDiffArry.push_back(cvPoint2D32f(maxIndex, diffVal[maxIndex]));
		posDiffArry.push_back(cvPoint2D32f(maxIndex-1, diffVal[maxIndex-1]));
		posDiffArry.push_back(cvPoint2D32f(maxIndex+1, diffVal[maxIndex+1]));
		lsm_order2(posDiffArry,  a,  b, c);
		posy=min-b/(2*a);
	}
	else
		posy=min+maxIndex;

	pos.x=posx;
	pos.y=posy;
	
	return maxDiff;
}

double ModelLinePosition::vertical_positon(const IplImage * img, double theta, CvPoint2D32f & pos){
	int i,j;
	double tanTheta=tan(theta);
	int offset;
	float point5=0.5;
	int widthStep = img->widthStep;
	unsigned char*headData=(unsigned char*)(img->imageData)+head*widthStep+min;
	double diff=0, maxDiff;
	int maxIndex;
//	printf("head=%d, tai=%d, min=%d, max=%d", head, tail, min, max);
	int add1=m_step;
	int add2=-(m_step-1);
	float posx=0, posy=head;

	if(tanTheta<0)
		point5=-0.5;
	
	for(i=0;i<len;i++){
		offset=i*tanTheta+point5;
		pointPtrs[i]=headData+i*widthStep+offset;
		for(j=1;j<m_step;j++)
			diff+=pointPtrs[i][j]-pointPtrs[i][-j];
	}
	maxDiff=diffVal[0]=fabs(diff);
	maxIndex=0;

	for(i=1; i<scanW; i++){
		for(j=0;j<len;j++){
			diff+=pointPtrs[j][add1]-pointPtrs[j][1]-(pointPtrs[j][0]-pointPtrs[j][add2]);
			pointPtrs[j]+=1;
		}
		diffVal[i]=fabs(diff);
		if(diffVal[i]>maxDiff){
			maxDiff=diffVal[i];
			maxIndex=i;
		}
	}	

	if(maxIndex!=0 && maxIndex!=scanW-1)
	{
		double a,b, c;
		posDiffArry.clear();
		posDiffArry.push_back(cvPoint2D32f(maxIndex, diffVal[maxIndex]));
		posDiffArry.push_back(cvPoint2D32f(maxIndex-1, diffVal[maxIndex-1]));
		posDiffArry.push_back(cvPoint2D32f(maxIndex+1, diffVal[maxIndex+1]));
		lsm_order2(posDiffArry,  a,  b, c);
		posx=min-b/(2*a);
	}
	else
		posx=min+maxIndex;
	
	pos.x=posx;
	pos.y=posy;
	
	return maxDiff;
}


Model2LinePosition::Model2LinePosition(LineType lineType, int step)
	:type(lineType), m_step(step), scanW(0), pointPtrs(NULL), diffVal(NULL){
	len[0]=0;
	len[1]=0;
	lenSum=0;
}

Model2LinePosition::Model2LinePosition(const Model2LinePosition & other){
	pointPtrs=NULL;
	diffVal=NULL;
	*this=other;
}

void Model2LinePosition::operator=(const Model2LinePosition & other){
	int i;
	type=other.type;
	m_step=other.m_step;
	for(i=0;i<2;i++){
		head[i]=other.head[i];
		tail[i]=other.tail[i];
		len[i]=other.len[i];
	}
	lenSum=other.lenSum;
	max=other.max;
	min=other.min;
	scanW=other.scanW;
	if(pointPtrs)
		free(pointPtrs);
	pointPtrs=(unsigned char**)malloc(lenSum*sizeof(unsigned char*));
	assert(pointPtrs);
	for(i=0;i<lenSum;i++)
		pointPtrs[i]=other.pointPtrs[i];
	if(diffVal)
		free(diffVal);
	diffVal=(double*)malloc(scanW*sizeof(double));
	assert(diffVal);
	for(i=0;i<scanW;i++)
		diffVal[i]=other.diffVal[i];
}



Model2LinePosition::~Model2LinePosition(){
	if(pointPtrs)
		free(pointPtrs);
	if(diffVal)
		free(diffVal);
}

void Model2LinePosition::set_range(int lineHead1, int lineTail1, int lineHead2, int lineTail2, 
	int minPos, int maxPos)
{
	assert(lineTail1>lineHead1);
	assert(lineHead2>lineTail1);
	assert(lineTail2>lineHead2);
	assert(maxPos>minPos);

	//printf("head=%d, tail=%d, min=%d, max=%d\n", head, tail, min, max);
	head[0]=lineHead1;
	tail[0]=lineTail1;
	head[1]=lineHead2;
	tail[1]=lineTail2;
	min=minPos;
	max=maxPos;
	len[0]=tail[0]-head[0]+1;
	len[1]=tail[1]-head[1]+1;
	int newLenSum=len[0]+len[1];
	
	if(newLenSum!=lenSum){
		lenSum=newLenSum;
		if(pointPtrs)
			free(pointPtrs);
		pointPtrs=(unsigned char**)malloc(lenSum*sizeof(unsigned char*));
		assert(pointPtrs);
	//	printf("allocate pointptrs\n");
	}
	
	int newScanW=max-min+1;
	if(newScanW!=scanW){
		scanW=newScanW;
		if(diffVal)
			free(diffVal);
		diffVal=(double*)malloc(scanW*sizeof(double));
		assert(diffVal);
	//	printf("allocate diffVal\n");
	}
}

double Model2LinePosition::figure_positon(const IplImage * img, double theta, CvPoint2D32f & pos){
	if(type==vertical)
		return vertical_positon(img, theta, pos);
	else
		return horizontal_positon( img,  theta, pos);
}



double Model2LinePosition::horizontal_positon(const IplImage * img, double theta, CvPoint2D32f & pos){
	int i,j;
	float tanTheta=-tan(theta);
	float point5=0.5;
	int widthStep = img->widthStep;
	unsigned char*headData=(unsigned char*)(img->imageData)+min*widthStep+head[0];
	double diff=0, maxDiff;
	int maxIndex;
	int add1=m_step*widthStep;
	int add2=-(m_step-1)*widthStep;
	float posx=head[0], posy=0;

	//printf("theta=%lf, start\n", theta);
	//printf("head=%d, tail=%d, min=%d, max=%d, len=%d, scanW=%d, widthStep=%d\n", head, tail, min, max, len, scanW, widthStep);
	if(tanTheta<0)
		point5=-0.5;

	unsigned char** pt=pointPtrs;
	float floatOffset=0;
	int intOffset=0;
	int len1=len[0];
	unsigned char *posPoint=NULL, *negPoint=NULL;
	for(i=0;i<len1;i++){
		intOffset=floatOffset+point5;
		*pt=headData+intOffset*widthStep+i;
		posPoint=(*pt)+widthStep;
		negPoint=(*pt)-widthStep;
		for(j=1;j<m_step;j++){
			diff+=*posPoint-*negPoint;
			posPoint+=widthStep;
			negPoint-=widthStep;
		}		
		floatOffset+=tanTheta;
		pt++;
	}

	floatOffset=(head[1]-head[0])*tanTheta;	
	int end2=tail[1]-head[0]+1;
	for(i=head[1]-head[0];i<end2;i++){
		intOffset=floatOffset+point5;
		*pt=headData+intOffset*widthStep+i;
		posPoint=(*pt)+widthStep;
		negPoint=(*pt)-widthStep;
		for(j=1;j<m_step;j++){
			diff+=*posPoint-*negPoint;
			posPoint+=widthStep;
			negPoint-=widthStep;
		}		
		floatOffset+=tanTheta;
		pt++;
	}
	
	maxDiff=diffVal[0]=fabs(diff);
	maxIndex=0;

	//printf("model line figure init ok\n");

	for(i=1; i<scanW; i++){
		for(j=0;j<lenSum;j++){
			diff+=pointPtrs[j][add1]-pointPtrs[j][widthStep]-(pointPtrs[j][0]-pointPtrs[j][add2]);
			pointPtrs[j]+=widthStep;
		}
		diffVal[i]=fabs(diff);
		if(diffVal[i]>maxDiff){
			maxDiff=diffVal[i];
			maxIndex=i;
		}
	}	
	//printf("model line figure  ok\n");

	if(maxIndex!=0 && maxIndex!=scanW-1)
	{
		double a,b, c;
		posDiffArry.clear();
		posDiffArry.push_back(cvPoint2D32f(maxIndex, diffVal[maxIndex]));
		posDiffArry.push_back(cvPoint2D32f(maxIndex-1, diffVal[maxIndex-1]));
		posDiffArry.push_back(cvPoint2D32f(maxIndex+1, diffVal[maxIndex+1]));
		lsm_order2(posDiffArry,  a,  b, c);
		posy=min-b/(2*a);
	}
	else
		posy=min+maxIndex;

	pos.x=posx;
	pos.y=posy;
	
	return maxDiff;
}


double Model2LinePosition::vertical_positon(const IplImage * img, double theta, CvPoint2D32f & pos){
	int i,j;
	double tanTheta=tan(theta);
	//int offset;
	float point5=0.5;
	int widthStep = img->widthStep;
	unsigned char*headData=(unsigned char*)(img->imageData)+head[0]*widthStep+min;
	double diff=0, maxDiff;
	int maxIndex;
//	printf("head=%d, tai=%d, min=%d, max=%d", head, tail, min, max);
	int add1=m_step;
	int add2=-(m_step-1);
	float posx=0, posy=head[0];

	if(tanTheta<0)
		point5=-0.5;


	int len1=len[0];
	float floatOffset=0;
	int intOffset=0;
	unsigned char** pt=pointPtrs;
	unsigned char *posPoint=NULL, *negPoint=NULL;
	for(i=0;i<len1;i++){
		intOffset=floatOffset+point5;
		*pt=headData+i*widthStep+intOffset;
		posPoint=*pt+1;
		negPoint=*pt-1;
		for(j=1;j<m_step;j++){
			diff+=*posPoint-*negPoint;
			posPoint++;
			negPoint--;
		}
		floatOffset+=tanTheta;
		pt++;
	}

	floatOffset=tanTheta*(head[1]-head[0]);
	int end2=tail[1]-head[0]+1;
	for(i=head[1]-head[0];i<end2;i++){
		intOffset=floatOffset+point5;
		*pt=headData+i*widthStep+intOffset;
		posPoint=*pt+1;
		negPoint=*pt-1;
		for(j=1;j<m_step;j++){
			diff+=*posPoint-*negPoint;
			posPoint++;
			negPoint--;
		}
		floatOffset+=tanTheta;
		pt++;
	}

	
	maxDiff=diffVal[0]=fabs(diff);
	maxIndex=0;

	for(i=1; i<scanW; i++){
		for(j=0;j<lenSum;j++){
			diff+=pointPtrs[j][add1]-pointPtrs[j][1]-(pointPtrs[j][0]-pointPtrs[j][add2]);
			pointPtrs[j]+=1;
		}
		diffVal[i]=fabs(diff);
		if(diffVal[i]>maxDiff){
			maxDiff=diffVal[i];
			maxIndex=i;
		}
	}	

	if(maxIndex!=0 && maxIndex!=scanW-1)
	{
		double a,b, c;
		posDiffArry.clear();
		posDiffArry.push_back(cvPoint2D32f(maxIndex, diffVal[maxIndex]));
		posDiffArry.push_back(cvPoint2D32f(maxIndex-1, diffVal[maxIndex-1]));
		posDiffArry.push_back(cvPoint2D32f(maxIndex+1, diffVal[maxIndex+1]));
		lsm_order2(posDiffArry,  a,  b, c);
		posx=min-b/(2*a);
	}
	else
		posx=min+maxIndex;
	
	pos.x=posx;
	pos.y=posy;
	
	return maxDiff;
}


void ImageAdjust::add(const Point & center, const Point & pos){
	centers.push_back(center);
	camPos.push_back(pos);
}

void ImageAdjust::clear(){
	centers.clear();
	camPos.clear();
}

bool ImageAdjust:: figure_up(TransformMatrix& tm)
{
	double kxx, kxy, kyx, kyy;
	bool sucess=figure_up(kxx, kxy, kyx, kyy);
	tm=TransformMatrix(kxx, kxy, kyx,kyy);
	return sucess;
}
bool ImageAdjust:: figure_up(double& kxx, double&kxy, double& kyx, double& kyy)
{
	if(centers.size()<4){
		printf("there is no enough adjustment data, at least 4 groups\n");
		clear();
		return false;
	}
	vector<Point>::iterator cenIt, posIt;
	cenIt=centers.begin();
	posIt=camPos.begin();
	
	double cx0=cenIt->x();
	double cy0=cenIt->y();
	double px0=posIt->x();
	double py0=posIt->y();
	
	double sum_xz1=0;
	double sum_xz2=0;
	double sum_yz1=0;
	double sum_yz2=0;
	double sum_xx=0;
	double sum_yy=0;
	double sum_xy=0;
	double x,y,z1,z2;
	for(cenIt++, posIt++;cenIt != centers.end(); cenIt++, posIt++){
		x=cenIt->x()-cx0;
		y=cenIt->y()-cy0;
		z1=-(posIt->x()-px0);
		z2=-(posIt->y()-py0);
		sum_xx+=x*x;
		sum_yy+=y*y;
		sum_xy+=x*y;
		sum_xz1+=x*z1;
		sum_yz1+=y*z1;
		sum_xz2+=x*z2;
		sum_yz2+=y*z2;
	}
	
	double den=sum_xx*sum_yy-sum_xy*sum_xy;
	kxx=(sum_xz1*sum_yy-sum_xy*sum_yz1)/den;
	kxy=(sum_xx*sum_yz1-sum_xz1*sum_xy)/den;
	kyx=(sum_xz2*sum_yy-sum_xy*sum_yz2)/den;
	kyy=(sum_xx*sum_yz2-sum_xz2*sum_xy)/den;
	clear();
	return true;
}



