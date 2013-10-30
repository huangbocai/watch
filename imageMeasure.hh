

/********************************************************************
* Description: imageMeasure.hh
* Head file for image measurement(not just for CVM), which contain image edge-recognizer,
* segment-synthesizer and geom-operater.
*
* Author: Huang Bocai, Chen Qiuqiang
* License:
* System: Linux
*
* Copyright (c) 2012 All rights reserved by Qiezhi.
********************************************************************/

#include <semaphore.h>
#include <vector>
#include <queue>
#include <cv.h>
using namespace std;

#ifndef IMAGE_MESURE_HH
#define IMAGE_MESURE_HH

#ifndef PI
#define PI   3.141592653589
#endif

#ifndef MORE_THAN 
#define MORE_THAN(a,b) (((a)-(b)>0.00001)? 1:0)
#endif

#ifndef LESS_THAN
#define LESS_THAN(a, b) (((b)-(a)>0.00001)? 1:0)
#endif

#ifndef EQU
#define EQU(a,b) ((!MORE_THAN((a), (b)))&&(!LESS_THAN((a), (b))))
#endif

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef Y_EQU_FX
#define Y_EQU_FX 0 //y=kx+b, used for top or bottom edge
#define X_EQU_FY 1 //x=ky+b, used for left or right edge
#endif


/*declarate classes of geom figures*/

namespace hbc{
class Vector2;

class GeomBase
{
public:
	GeomBase(): dataNum(0){};
	GeomBase(double v0);
	GeomBase(double v0, double v1);
	GeomBase(double v0, double v1, double v2);
	GeomBase(double v0, double v1, double v2, double v3);
	GeomBase(double v0, double v1, double v2, double v3, double v4);
	double& operator[](int i);
	int size() const{return dataNum;};
    virtual void move(const Vector2&){};
	static const int GEOM_BASE_DATA_MAX=5;
	friend bool operator==(const GeomBase& gb1, const GeomBase& gb2);
	friend bool operator!=(const GeomBase& gb1, const GeomBase& gb2);
protected:
	int dataNum;
	double data[GEOM_BASE_DATA_MAX];
};


class Point : public GeomBase
{
public:
	Point(double x=0, double y=0): GeomBase(x, y){};
	double x()const {return data[0];};
	double y()const {return data[1];};
	void set(double x, double y){data[0]=x, data[1]=y;};
	void move(const Vector2& v);
};

class Vector2
{
public:
	Vector2(double x=0, double y=0):vx(x), vy(y){};
	Vector2(const Point& start, const Point& end);
	double length()const;
	void zoom(double ratio);
	void rotate(double angle);
	void set(double x, double y){vx=x, vy=y;};
	double x()const {return vx;};
	double y()const {return vy;};
private:
	double vx;
	double vy;
};



class Line: public GeomBase
{
public:
	Line(double x1=0, double y1=0, double x2=0, double y2=0):GeomBase(x1, y1, x2,y2){};
	Line(const Point& p1, const Point& p2):GeomBase(p1.x(), p1.y(), p2.x(), p2.y()){};
	Point p1()const {return Point(data[0], data[1]);};
	Point p2()const {return Point(data[2], data[3]);};
	double x1()const {return data[0];};
	double y1()const {return data[1];};
	double x2()const {return data[2];};
	double y2()const {return data[3];};
	void set_x1(double v){data[0]=v;};
	void set_y1(double v){data[1]=v;};
	void set_x2(double v){data[2]=v;};
	void set_y2(double v){data[3]=v;};
	void set(double x1, double y1, double x2, double y2)
		{data[0]=x1; data[1]=y1; data[2]=x2;data[3]=y2;};
	double length() const;
	void move(const Vector2& v);
};

typedef Line TwoPoint;

class Circle: public GeomBase
{
public:
	Circle(const Point& c, double r):GeomBase(c.x(), c.y(), r){};
	Circle(double cx=0, double cy=0, double r=0):GeomBase(cx, cy, r){};
	Circle(const Point&  p1, const Point& p2, const Point& p3);
	Circle(double x1, double y1, double x2, double y2, double x3, double y3);
	void set(double x1, double y1, double x2, double y2, double x3, double y3);
	Point centre()const {return Point(data[0], data[1]);};
	double cx() const{return data[0];};
	double cy() const{return data[1];};
	double r() const {return data[2];};
	void move(const Vector2& v);
};

class Arc: public GeomBase
{
public:
	Arc(const Point& c, double r, double start, double end);
	Arc(double cx=0, double cy=0, double r=0, double start=0, double end=0);
	Arc(const Point&  p1, const Point& p2, const Point& p3);
	Arc(double x1, double y1, double x2, double y2, double x3, double y3);
	void set(double x1, double y1, double x2, double y2, double x3, double y3);
	Point centre()const {return Point(data[0], data[1]);};
	double cx() const{return data[0];};
	double cy() const{return data[1];};
	double r()const {return data[2];};
	double start()const {return data[3];};
	double end()const {return data[4];};
	void move(const Vector2& v);
};

class Distance: public GeomBase
{
public:
	Distance(double value=0):GeomBase(value, value, value){};
	Distance(double averge, double max, double min):GeomBase(averge, max, min){};
	double averge() const {return data[0];};
	double max() const{return data[1];};
	double min() const{return data[2];};
};

class Angle: public GeomBase
{
public:
	Angle(double rad=0):GeomBase(rad){};
	double radian() const{return data[0];};
	double degree() const{return data[0]/PI*180;};
};


/**/
class GeomElement
{
public: 
	typedef enum {
		ANY=0,
		POINT,
		LINE,
		CIRCLE,
		ARC,
		DISTANCE,
		ANGLE
		}GeomType;
	GeomElement( bool valid=true);
	GeomElement(const Point& point, bool valid=true);
	GeomElement(const Line& line,  bool valid=true);
	GeomElement(const Circle& circle,  bool valid=true);
	GeomElement(const Arc& arc, bool valid=true);
	GeomElement(const Distance& distance,  bool valid=true);
	GeomElement(const Angle& angle, bool valid=true);
	GeomElement(const GeomElement& ge);
	GeomElement& operator=(const GeomElement& ge);
	double operator[](int n) const {return (*ptr)[n];}
	GeomType type() const {return m_type;};
	bool valid() const {return m_valid;};
	const Point&  get_point() const;
	const Line& get_line() const;
	const Circle& get_circle() const;
	const Arc& get_arc() const;
	const Distance& get_distance() const;
	const Angle& get_angle() const;
	void move(const Vector2& v);
	virtual ~GeomElement();
private:
	 bool m_valid;
	 GeomType m_type;
	 GeomBase* ptr;
};




/*declarate image measure command*/


//box selector
class Box{
public:
	//Box(const Point& p1, const Point& p2);
	Box(int x1=0, int y1=0, int x2=0, int y2=0):p1(x1, y1), p2(x2,y2){};
	void set(int x1, int y1, int x2, int y2);
	int left() const;
	int top() const;
	int right() const;
	int  bottom() const;
private:
	Point p1;
	Point p2;
};


class BoxSelector{
public:
	typedef enum{
		NO_DIR,
		UP,
		DOWN,
		LEFT,
		RIGHT,
	}Direction;
	BoxSelector(int x1, int y1, int x2, int y2);
	BoxSelector(int x1=0, int y1=0, int x2=0, int y2=0, Direction direction=NO_DIR):m_box(x1,y1,x2,y2), m_direction(direction){};
	void set(int x1, int y1, int x2, int y2);
	int left() const{return m_box.left();};
	int right() const{return m_box.right();};
	int top() const{return m_box.top();};
	int bottom() const{return m_box.bottom();};
	const Box& box() const {return m_box;};
	Direction get_direction() const {return m_direction;};
	Direction& direction(){return m_direction;};
private:
	Box m_box;
	Direction m_direction;
};

class PointSelector
{
public:
	PointSelector(int x1=0, int y1=0, int x2=0, int y2=0):m_line(x1, y1, x2, y2){};
	void set(int x1, int y1, int x2, int y2);
	const Line& line() const{return m_line;};
	int x1() const{return m_line.x1()+0.5;};
	int y1() const{return m_line.y1()+0.5;};
	int x2() const{return m_line.x2()+0.5;};
	int y2() const{return m_line.y2()+0.5;};
private:
	Line m_line;
};


//line selector
class LineSelector{
public:
	typedef enum{
		LEFT,
		RIGHT
	} Direction;
	//LineSelector(const Point& p1, const Point& p2, double width):m_line(p1, p2), m_width(width){};
	LineSelector(int x1=0, int y1=0, int x2=0, int y2=0, double width=0, Direction direction=LEFT)
		: m_line(x1, y1, x2, y2), m_width(width), m_direction(direction){};
	void set(int x1, int y1, int x2, int y2, int width);
	double  get_width()const {return m_width;};
	double& width(){return m_width;};
	Direction get_direction() const {return m_direction;};
	Direction& direction(){return m_direction;};
	const Line& line(){return m_line;};
	int x1() const{return m_line.x1()+0.5;};
	int y1() const{return m_line.y1()+0.5;};
	int x2() const{return m_line.x2()+0.5;};
	int y2() const{return m_line.y2()+0.5;};
	
private:
	Line m_line;
	double m_width;
	Direction m_direction;
};

class CircleSelector{
public:
	typedef enum{
		INSIDE,
		OUTSIDE,
	}Direction;
	//CircleSelector(const Point&  p1, const Point& p2, const Point& p3, double width):m_circle(p1, p2,p3), m_width(width){};
	CircleSelector(double x1, double y1, double x2, double y2,double x3, double y3, 
		double width=0, Direction direction=INSIDE):m_circle(x1,y1,x2,y2,x3,y3),m_width(width),m_direction(direction){};
	CircleSelector(double cx=0, double cy=0, double r=0, double width=0, Direction direction=INSIDE):m_circle(cx, cy, r), m_width(width), m_direction(direction){};
	void set(int x1, int y1, int x2, int y2, int x3, int y3, int width);
	double r() const {return m_circle.r();};
	double cx() const {return m_circle.cx();};
	double cy() const {return m_circle.cy();};
	double& width(){return m_width;};
	double  get_width()const {return m_width;};
	Direction& direction(){return m_direction;};
	Direction get_direction() const {return m_direction;};
private:
	Circle m_circle;
	double m_width;
	Direction m_direction;
};


class ArcSelector{
public:
	typedef enum{
		INSIDE,
		OUTSIDE,
	}Direction;
	ArcSelector(double cx=0, double cy=0, double r=0, double start=0, double end=0,  int width=0, Direction dirction=INSIDE)
		:m_arc(cx, cy, r, start, end),m_width(width), m_direction(dirction){};
	ArcSelector(int x1, int y1, int x2, int  y2, int x3, int y3, int width=0, Direction dirction=INSIDE)
	:m_arc(x1, y1, x2, y2, x3, y3), m_width(width), m_direction(dirction){};
	void set(int x1, int y1, int x2, int y2, int x3, int y3, int width);
	double r() const {return m_arc.r();};
	double cx() const {return m_arc.cx();};
	double cy() const {return m_arc.cy();};
	double start() const {return m_arc.start();};
	double end() const {return m_arc.end();};
	double get_width() const {return m_width;};
	double& width() {return m_width;};
	Direction get_direction() const {return m_direction;};
	Direction& direction()  {return m_direction;};
	
private:
	Arc m_arc;
	double m_width;
	Direction m_direction;
};

//image measure command
class IMC  
{
public:
	static const int maxDepend=10;
	typedef enum{
		NO_CMD=0,
		//recognize cmd
		LINE_IN_BOX=1,
		CIRCLE_IN_BOX,
		ARC_IN_BOX,
		POINT_IN_LINE,
		LINE_IN_LINE,
		CIRCLE_IN_LINE,
		ARC_IN_LINE,
		TANGENT_LINE,
		
		//combination cmd
		COMB_LINE=11,
		COMB_CIRCLE,
		COMB_ARC,
		
		//geom cmd
		INTERSECTION_POINT=21,
		MID_POINT,
		TWO_POINT_LINE,
		MID_LINE,
		TWO_POINT_DISTANCE,
		POINT_LINE_DISTANCE,
		TWO_LINE_DISTANCE,
		TWO_LINE_ANGLE,
		PERPENDICULAR_LINE,
		TWO_CIRCLE_DISTANCE,
		CIRCLE_LINE_DISTANCE,
		EXTEND_LINE,
		CIRCLE_LINE_INTERSETION,
		TWO_CIRCLE_TANGENT,
		TWO_CIRCLE_MID_LINE,
		TWO_CIRCLE_INTERSETION,
		
		//measure end
		MEASURE_END=99,

		ADJUST_CIRCLE_IN_BOX
	}CmdType;
	IMC():m_cmdType(NO_CMD),selector(NULL){};
	//recognize cmd
	IMC(CmdType cmdType, const BoxSelector& box);
	IMC(const PointSelector& pointSelector);
	IMC(const LineSelector& lineSelector);
	IMC(const CircleSelector& circleSelector);
	IMC(const ArcSelector& arcSelector);
	IMC(const LineSelector& lineSelector, int depend1); //use to get tangent
	IMC(CmdType cmdType, int depend1);
	IMC(CmdType cmdType, int depend1, const Point& selPos1);
	IMC(CmdType cmdType, int depend1, int depend2);
	IMC(CmdType cmdType, int depend1, int depend2, const Point& selPos1);
	IMC(CmdType cmdType, int depend1, int depend2, const Point& selPos1, const Point& selPos2);
	IMC(CmdType cmdType, const vector<int> & depends);
	IMC(const IMC& imc);
	const IMC& operator=(const IMC& imc);
	virtual ~IMC();
	CmdType cmd_type() const{return m_cmdType;};
	const vector<int >&  depend() const {return m_depend;};
	const Point& sel_pos1()const {return selPos[0];};
	const Point& sel_pos2()const{return selPos[1];};
	const BoxSelector& box_selector() const;
	const PointSelector& point_selector() const;
	const LineSelector& line_selector() const;
	const CircleSelector& circle_selector() const;
	const ArcSelector& arc_selector() const;
	static const char* cmd_name(CmdType type);
	static bool is_cv_cmd(CmdType type);
	static bool is_comb_cmd(CmdType type);
	static bool is_geom_cmd(CmdType type);
	static GeomElement::GeomType result_type(CmdType type);
private:
	void delete_selector();
	Point selPos[2];
	CmdType m_cmdType;
	void* selector;
	vector<int> m_depend;
};


class TransformMatrix
{
public:
	TransformMatrix(double kxx=1, double kxy=0, double kyx=0, double kyy=1);
	void transform(double srcx, double srcy, double &detx, double& dety) const;
	void inv_transform(double srcx, double srcy, double &detx, double& dety) const;
	double kx() const{return m_kxx;};
	double ky() const{return m_kyy;};
    double in_kx() const{return in_kxx;};
	double k() const{return (m_kxx+m_kyy)/2;}
	double rotate_angle()const {return rotateAngle;};
protected:
	double m_kxx;
	double m_kxy;
	double m_kyx;
	double m_kyy;
	double in_kxx;
	double in_kxy;
	double in_kyx;
	double in_kyy;
	double rotateAngle;
};

class ImageActualTM: public TransformMatrix
{
public:
	ImageActualTM(const TransformMatrix& transformMatrix, int imageWidth, int imageHeight, 
		double centerX=0, double centerY=0)
		:TransformMatrix(transformMatrix), halfWidth(imageWidth/2), halfHeight(imageHeight/2), cx(centerX), cy(centerY){};
	void set_center_pos(double centerX, double centerY){cx=centerX; cy=centerY;};
	Point get_center_pos() const {return Point(cx, cy);};
	ImageActualTM& operator=(const TransformMatrix& tm);
	Point transform(double imgX, double imgY) const;
	Point inv_transform(const Point& actual) const;
	Point inv_transform(double x, double y) const;
private:
	int halfWidth;
	int halfHeight;
	double cx;
	double cy;
};


class ValidPoint{
public:
	ValidPoint(double vx=0, double vy=0, bool isValid=true):x(vx),y(vy),valid(isValid){};
	double x;
	double y;
	bool valid;
};

class PointsArray  
{
public:
	PointsArray(int n=0){ps.reserve(n);};
	PointsArray(const PointsArray& pa);
	void add(double x, double y);
	void add(const Point& point);
	void add(const PointsArray& addend);
	void add(const ValidPoint& point);
	void sample(unsigned int sampleNum);
	int valid_num()const;
	int size()const {return ps.size();};
	void clear(){ps.clear();};
	ValidPoint& operator[](int index){return ps[index];};
	void move(const Vector2& v);
	void valid_only();
	const ValidPoint& first_valid() const;
	const ValidPoint& last_valid() const;
	vector<ValidPoint>::iterator begin(){return ps.begin();};
	vector<ValidPoint>::iterator end(){return ps.end();};
private:
	vector<ValidPoint> ps;
};


class EdgeRecognizer{
public:
	EdgeRecognizer(const TransformMatrix& transformMatrix, float roughRmDistance,
	float lsmRmDistance, int gradientThresh=64, int grayThresh=128);
	GeomElement recognize(const IplImage* image, const IMC& imc, const GeomElement& ge=GeomElement());
	PointsArray& point_array(){return points;};
	void set_edge_thresh(int val){gradientTh=val;};
	void set_transform_matrix(const TransformMatrix& transformMatrix){tm=transformMatrix;};
	virtual ~EdgeRecognizer();
private:
	void scan_edge_in_box_selector(const BoxSelector& boxSltr);
	void  circle_edge_in_box_selector(const BoxSelector& boxSltr);
	bool scan_edge_in_line(const PointSelector & pointSltr, double& px, double&py);
	bool scan_no_stain_edge_in_line(const PointSelector & pointSltr, double& px, double&py, int stainSize=4);
	void scan_edge_in_line_selector(const LineSelector& lineSltr,  int stainSize=0);
	void scan_edge_in_circle_selector(const CircleSelector& circleSltr);
	void scan_edge_in_arc_selector(const ArcSelector& arcSltr, bool isCircle=false);
	Line line_in_box(const BoxSelector& boxSltr, bool& sucess);
	Circle circle_in_box(const BoxSelector& boxSltr, bool& sucess);
	Point point_in_line(const PointSelector& pontSltr, bool& sucess);
	Line line_in_line(const LineSelector& lineSltr, bool& sucess);
	Circle circle_in_line(const CircleSelector& circleSltr, bool& sucess);
	Arc arc_in_line(const ArcSelector& arcSltr, bool& sucess);
	Line tangent_line(const LineSelector& lineSltr, const Line& parallel, bool& sucess);
	IplImage* smooth;
	TransformMatrix tm;
	PointsArray points;
	//PointsArray nearPoints;
	int gradientTh;
	int grayTh;
	float roughRmDis;
	float lsmRmDis;
	static const int maxPoints=300;//to avoid overrflow in lsm
};

/*
class MarkRecognizer
{
public:
	typedef enum{
		BLACK_CROSS,
		WHITE_CROSS,
		BLACK_L,
		WHITE_L
	}MarkType;
	MarkRecognizer();	
	Point recognize(const IplImage* image, MarkType markType, double param1, double param2);
	virtual ~MarkRecognizer();
private:
	IplImage* smooth;
	TransformMatrix tm;
	float roughRmDis;
	float lsmRmDis;
};*/

class GeomRelationship{
public:
	//GeomRelationship();
	GeomElement figure_up(const IMC& imc, const GeomElement& g1);
	GeomElement figure_up(const IMC& imc, const GeomElement& g1, const GeomElement& g2);
//private:
};

class EdgeGeomCombination{
public:
	void add(const PointsArray& pointArray);
	GeomElement figure_up(const IMC& imc);
private:
	Line comb_line(bool& sucess);
	Circle comb_circle(bool& sucess);
	Arc comb_arc(bool& sucess);
	PointsArray points;
	PointsArray terminals;
	static const int maxPoints=300;//to avoid overrflow in lsm
};

class ImageAdjust
{
public:
	void add(const Point &center, const Point &pos);
	void clear();
	bool figure_up(double& kxx, double&kxy, double& kyx, double& kyy);
	bool figure_up(TransformMatrix& tm);
	int size() const{return centers.size();};
private:
	vector<Point> centers;
	vector<Point> camPos;
};

class HoughLine{
public:
	HoughLine(float thetaVal=0, float rhoVal=0):theta(thetaVal), rho(rhoVal){};
	float theta;
	float rho;
};

class HoughTransformer
{
public:
	HoughTransformer(double rho, double theta, double maxRho, double thetaBegin, double thetaEnd, int linesNumMax, Point center);
	//HoughTransformer(float rho, float theta, float maxRho);
	void get_hough_lines(const vector <Point> & points,vector <HoughLine> & lines, int threshold) const;
	~HoughTransformer();
private:
	int area_accum(int angleIndex, int rhoIndex) const;
	float m_rho;
	float m_theta;
	float m_thetaBegin;
	int  linesMax;
	Point m_center;
	int numangle, numrho, numrho2;
	float* tabSin;
	float* tabCos;	
	int* accum;
	long* sortAccum;
};

class MarkPosition
{
public:
	MarkPosition(const TransformMatrix& transformMatrix, float roughRmDistance,
	float lsmRmDistance, int gradientThresh=64, int grayThresh=128);
	void set_transform_matrix(const TransformMatrix& transformMatrix);
	//virtual Point figure_position(const IplImage*image){};
	~MarkPosition();
protected:
	void prepare_process(const IplImage*image);
	IplImage*smooth;
	IplImage*bitmap;
	TransformMatrix tsfm;
};

class CrossMarkPosition:public MarkPosition
{
public: 
		typedef enum{BALCK, WHITE} Color;
		CrossMarkPosition();
		void set_length(float l);
		void set_width(float w);
		void set_color(Color color);
		Point figure_position(const IplImage*image);
		Point figure_position(const IplImage*image, const BoxSelector& boxSelector);
private:
		float lenth, width;
		Color color;
};


class ModelLinePosition{
	public:
		typedef enum{vertical, horizontal}LineType;
		ModelLinePosition(LineType lineType, int step=20);
		ModelLinePosition(const ModelLinePosition& other);
		void operator=(const ModelLinePosition& other);
		void set_step(int step){m_step=step;};
		void set_range(int lineHead, int lineTail, int minPos, int maxPos);
		double figure_positon(const IplImage * img, double theta, CvPoint2D32f & pos);
		~ModelLinePosition();
	private:
		double horizontal_positon(const IplImage * img, double theta, CvPoint2D32f & pos);
		double vertical_positon(const IplImage * img, double theta, CvPoint2D32f& pos);
		LineType type;
		int m_step;
		int head,tail;
		int max, min;
		int len, scanW;
		unsigned char** pointPtrs;
		double* diffVal;
		vector<CvPoint2D32f> posDiffArry;
};

class Model2LinePosition{
	public:
		typedef enum{vertical, horizontal}LineType;
		Model2LinePosition(LineType lineType, int step=20);
		Model2LinePosition(const Model2LinePosition& other);
		void operator=(const Model2LinePosition& other);
		void set_step(int step){m_step=step;};
		void set_range(int lineHead1, int lineTail1, int lineHead2, int lineTail2, int minPos, int maxPos);
		double figure_positon(const IplImage * img, double theta, CvPoint2D32f & pos);
		~Model2LinePosition();
	private:
		double horizontal_positon(const IplImage * img, double theta, CvPoint2D32f & pos);
		double vertical_positon(const IplImage * img, double theta, CvPoint2D32f& pos);
		LineType type;
		int m_step;
		int head[2],tail[2];
		int max, min;
		int len[2], lenSum, scanW;
		unsigned char** pointPtrs;
		double* diffVal;
		vector<CvPoint2D32f> posDiffArry;
};


/*some geom function*/
int toInt(double x);

double in_bound(double data, double min, double max);

double in_2pi(double angle);

//return rad value in [0,2PI)
double arctan(double x, double y);

bool in_angle_range(double angle, double start, double end);

double inner_product(const Vector2& v1, const Vector2& v2);

double outer_product(const Vector2& v1, const Vector2& v2);

double vector_lenght(const Vector2& vct);

Vector2 vector_rotate(const Vector2& vct, double angle);

double two_points_distance(const Point& p1, const Point& p2);

Point mid_point(const Point& p1, const Point& p2);

Point mid_point(const Line& l);

Line mid_line(const Line& l1, const Line& l2);

Line perpendicular_line(const Line& line);

double point_line_distance(const  Point& p, const Line& l);

Distance two_lines_distance(const  Line& l1, const Line& l2);

double two_vectors_angle(const Vector2& v1, const Vector2 v2);

 Angle two_lines_angle(const Line& l1, const Line& l2);

void  two_lines_intersection(const Line& l1, const Line& l2, double& x, double& y);

const Point two_lines_intersection(const Line& l1, const Line& l2);

bool three_points_circle(const Point& p1, const Point& p2, const Point& p3, double& cx, double& cy, double& r);

Point nearest_point_in_line(const Point& p, const Line& l);

bool circle_line_interserction(const Line& l, const Circle& c, Point& p1, Point& p2);

bool arc_line_interserction(const Line& l, const Arc& arc, Point& p1, Point& p2);

bool point_circle_tangent(const Point& p, const Circle& c, Line& l1, Line& l2);

bool two_circles_tangent(const Circle& c1, const Circle& c2, vector<Line>& tangents);

bool two_circles_intersection(const Circle& c1, const Circle& c2, Point& p1, Point& p2);

int vertical_modle_line_position(IplImage* img, int step, float theta, int begin, int end, double& diffVal, double* diffs=NULL);

int  horizontal_modle_line_position(IplImage* img, int step, float theta, int begin, int end, double& diffVal, double* diffs=NULL);

void  lsm_order2(const vector<CvPoint2D32f> & xyArry, double& a, double& b, double& c);
}
#endif




