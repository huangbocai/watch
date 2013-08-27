#ifndef WATCHDETECT_H
#define WATCHDETECT_H

#include <cv.h>
#include <list>
#include <vector>
#include "imageMeasure.hh"
using namespace hbc;
//class CirclesDetecter{
//public:
//    CirclesDetecter();
//    void set_figure(int gray, float len, float r);
//    int detect(const IplImage* img);
//    void set_pattern(IplImage* img, const CvRect* area=NULL);
//    void set_area(CvRect*area=NULL);

//    const list<Point>& get_positions()const {return positions;}
//    const IplImage* get_pattern()const{return pattern;}
//    bool pattern_is_new();

//    ~CirclesDetecter();
//private:
//    static const int threshold=200;
//    void learn_pattern();
//    IplImage* bitImage;
//    float length;
//    float radious;
//    IplConvKernel* kernel;
//    CvMemStorage* storage;
//    CvSeq*contours;
//    list<Point> positions;

//    IplImage* pattern;
//    bool patternIsNew;

//    CvRect* validArea;
//};




class WatchCircleDetecter{
public:
    WatchCircleDetecter();
    void set_pattern(IplImage* img, const CvRect* area=NULL);
    virtual const list<Point>& detect(IplImage* image, CvRect* roi=NULL);
    void set_similar(double val){m_threshold=val;}
    void set_level(int val){m_topLevel=val;}
    IplImage* createIdealPattern(float R, float outWidth);

    const list<Point>& get_positions()const {return m_centers;}
    const IplImage* get_pattern()const{return m_patternImage;}
    float radious()const {return m_R;}
    bool pattern_is_new();


    ~WatchCircleDetecter();
protected:
    static const int maxLevel=10;
    static const int blockRange=5;
    IplImage* m_pattern[maxLevel];
    IplImage* m_roiImage[maxLevel];
    list<Point> m_centers;
    CvRect m_roi;
private:
    void setCirclePattern(const IplImage* pattern, float R, float outWidth=5);
    bool m_patternIsNew;
    IplImage* m_patternImage;
    int level;
    IplImage* m_convolution[maxLevel];
    IplImage* m_blockImage[maxLevel];

    float m_R;
    double m_threshold;

    int m_topLevel;
};

class DiamondCircleDetecter : public WatchCircleDetecter
{
public:
     IplImage* createIdealPattern(float R, float outWidth=0);
     const list<Point>& detect(IplImage* image, CvRect* roi=NULL);
};

#endif // WATCHDETECT_H
