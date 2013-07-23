#ifndef WATCHDETECT_H
#define WATCHDETECT_H

#include <cv.h>
#include <list>
#include <vector>
#include "imageMeasure.hh"

class CirclesDetecter{
public:
    CirclesDetecter();
    void set_figure(int gray, float len, float r);
    int detect(const IplImage* img);
    void set_pattern(IplImage* img, const CvRect* area=NULL);
    void set_area(CvRect*area=NULL);

    const list<Point>& get_positions()const {return positions;}
    const IplImage* get_pattern()const{return pattern;}
    bool pattern_is_new();

    ~CirclesDetecter();
private:
    static const int threshold=200;
    void learn_pattern();
    IplImage* bitImage;
    float length;
    float radious;
    IplConvKernel* kernel;
    CvMemStorage* storage;
    CvSeq*contours;
    list<Point> positions;

    IplImage* pattern;
    bool patternIsNew;

    CvRect* validArea;
};


class WatchCircleDetecter{
public:
    WatchCircleDetecter();
    void set_pattern(IplImage* img, const CvRect* area=NULL);
    const list<Point>& detect(IplImage* image, CvRect* roi=NULL);

    const list<Point>& get_positions()const {return m_centers;}
    const IplImage* get_pattern()const{return m_patternImage;}
    float radious()const {return m_R;}
    bool pattern_is_new();


    ~WatchCircleDetecter();

private:
     void setCirclePattern(const IplImage* pattern, float R, int outWidth=5, float likelihood=0.6);
     bool m_patternIsNew;
     IplImage* m_patternImage;
     CvMat* m_pattern;
     IplImage* m_roiImage;
     IplImage* m_convolution;
     CvRect m_roi;
     float m_R;
     double m_threshold;
     list<Point> m_centers;
};



#endif // WATCHDETECT_H
