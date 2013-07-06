#ifndef WATCHDETECT_H
#define WATCHDETECT_H

#include <cv.h>
#include <list>
#include "imageMeasure.hh"

class CirclesDetecter{
public:
    CirclesDetecter();
    void set_figure(int gray, float len, float r);
    int detect(const IplImage* img);
    void set_pattern(IplImage* img, const CvRect* area=NULL);
    void set_area(CvRect*area=NULL);

    const CvSeq* get_contour()const {return contours;}
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



#endif // WATCHDETECT_H
