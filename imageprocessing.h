#ifndef IMAGEPROCESSING_H
#define IMAGEPROCESSING_H

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <vector>

class CirclesFinder
{
public:
    CirclesFinder() :m_minDist(2), m_cannyThreshold(0), m_minVotes(0), m_minRadius(0), m_maxRadius(0){}
    CirclesFinder(double minDist, double cannyThreshold, double minVotes, int minRadius, int maxRadius)
        :m_minDist(minDist), m_cannyThreshold(cannyThreshold), m_minVotes(minVotes),
          m_minRadius(minRadius), m_maxRadius(maxRadius)
    {}

    //~CirclesFinder(){};

    //The source image must be 8-bit, single-channel
    std::vector<cv::Vec3f> findCircles(cv::Mat& src)
    {
        m_circles.clear();
        cv::GaussianBlur(src,src,cv::Size(5,5),1.5);
        cv::HoughCircles(src, m_circles, CV_HOUGH_GRADIENT,
            2,
            m_minDist,
            m_cannyThreshold,
            m_minVotes,
            m_minRadius,
            m_maxRadius);
        //std::cout<<"Circles num: "<<m_circles.size()<<std::endl;
        return m_circles;
    }

    void drawDetectedCircles(cv::Mat &image, cv::Scalar color = cv::Scalar(255,255,255))
    {
        std::vector<cv::Vec3f>::const_iterator itc = m_circles.begin();
        while(itc != m_circles.end())
        {
            cv::circle(image,
                       cv::Point((*itc)[0], (*itc)[1]),
                       (*itc)[2],
                       color,
                       2);
            ++itc;
        }
    }

    void setMinDist(double minDist)
    {
        m_minDist = minDist;
    }

    void setCannyHighThreshold(double threshold)
    {
        m_cannyThreshold = threshold;
    }

    void setMinVotes(double votes)
    {
        m_minVotes = votes;
    }

    void setMinRadius(int minRadius)
    {
        m_minRadius = minRadius;
    }

    void setMaxRadius(int maxRadius)
    {
        m_maxRadius = maxRadius;
    }

    /* data */
private:
    cv::Mat m_img;
    std::vector<cv::Vec3f> m_circles;
    double m_minDist;
    double m_cannyThreshold;
    double m_minVotes;
    int    m_minRadius;
    int    m_maxRadius;
};

#endif // IMAGEPROCESSING_H
