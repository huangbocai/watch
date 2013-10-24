#ifndef IMAGEPROCESSING_H
#define IMAGEPROCESSING_H

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>

class Histogram1D {

  private:

    int histSize[1];
    float hranges[2];
    const float* ranges[1];
    int channels[1];

  public:

    Histogram1D() {

        // Prepare arguments for 1D histogram
        histSize[0]= 256;
        hranges[0]= 0.0;
        hranges[1]= 255.0;
        ranges[0]= hranges;
        channels[0]= 0; // by default, we look at channel 0
    }

    // Sets the channel on which histogram will be calculated.
    // By default it is channel 0.
    void setChannel(int c) {

        channels[0]= c;
    }

    // Gets the channel used.
    int getChannel() {

        return channels[0];
    }

    // Sets the range for the pixel values.
    // By default it is [0,255]
    void setRange(float minValue, float maxValue) {

        hranges[0]= minValue;
        hranges[1]= maxValue;
    }

    // Gets the min pixel value.
    float getMinValue() {

        return hranges[0];
    }

    // Gets the max pixel value.
    float getMaxValue() {

        return hranges[1];
    }

    // Sets the number of bins in histogram.
    // By default it is 256.
    void setNBins(int nbins) {

        histSize[0]= nbins;
    }

    // Gets the number of bins in histogram.
    int getNBins() {

        return histSize[0];
    }

    // Computes the 1D histogram.
    cv::MatND getHistogram(const cv::Mat &image) {

        cv::MatND hist;

        // Compute histogram
        cv::calcHist(&image,
            1,			// histogram of 1 image only
            channels,	// the channel used
            cv::Mat(),	// no mask is used
            hist,		// the resulting histogram
            1,			// it is a 1D histogram
            histSize,	// number of bins
            ranges		// pixel value range
        );

        return hist;
    }

    // Computes the 1D histogram and returns an image of it.
    cv::Mat getHistogramImage(const cv::Mat &image){

        // Compute histogram first
        cv::MatND hist= getHistogram(image);

        // Get min and max bin values
        double maxVal=0;
        double minVal=0;
        cv::minMaxLoc(hist, &minVal, &maxVal, 0, 0);

        // Image on which to display histogram
        cv::Mat histImg(histSize[0], histSize[0], CV_8U,cv::Scalar(255));

        // set highest point at 90% of nbins
        int hpt = static_cast<int>(0.9*histSize[0]);

        // Draw vertical line for each bin
        for( int h = 0; h < histSize[0]; h++ ) {

            float binVal = hist.at<float>(h);
            int intensity = static_cast<int>(binVal*hpt/maxVal);
            cv::line(histImg,cv::Point(h,histSize[0]),cv::Point(h,histSize[0]-intensity),cv::Scalar::all(0));
        }

        return histImg;
    }

    // Equalizes the source image.
    cv::Mat equalize(const cv::Mat &image) {

        cv::Mat result;
        cv::equalizeHist(image,result);

        return result;
    }

    // Stretches the source image.
    cv::Mat stretch(const cv::Mat &image, int minValue=0) {

        // Compute histogram first
        cv::MatND hist= getHistogram(image);

        // find left extremity of the histogram
        int imin= 0;
        for( ; imin < histSize[0]; imin++ ) {
            std::cout<<hist.at<float>(imin)<<std::endl;
            if (hist.at<float>(imin) > minValue)
                break;
        }

        // find right extremity of the histogram
        int imax= histSize[0]-1;
        for( ; imax >= 0; imax-- ) {

            if (hist.at<float>(imax) > minValue)
                break;
        }

        // Create lookup table
        int dims[1]={256};
        cv::MatND lookup(1,dims,CV_8U);

        for (int i=0; i<256; i++) {

            if (i < imin) lookup.at<uchar>(i)= 0;
            else if (i > imax) lookup.at<uchar>(i)= 255;
            else lookup.at<uchar>(i)= static_cast<uchar>(255.0*(i-imin)/(imax-imin)+0.5);
        }

        // Apply lookup table
        cv::Mat result;
        result= applyLookUp(image,lookup);

        return result;
    }

    // Applies a lookup table transforming an input image into a 1-channel image
    cv::Mat applyLookUp(const cv::Mat& image, const cv::MatND& lookup) {

        // Set output image (always 1-channel)
        cv::Mat result(image.rows,image.cols,CV_8U);
        cv::Mat_<uchar>::iterator itr= result.begin<uchar>();

        // Iterates over the input image
        cv::Mat_<uchar>::const_iterator it= image.begin<uchar>();
        cv::Mat_<uchar>::const_iterator itend= image.end<uchar>();

        // Applies lookup to each pixel
        for ( ; it!= itend; ++it, ++itr) {

            *itr= lookup.at<uchar>(*it);
        }

        return result;
    }
};

class ColourHistogram {

  private:

    int histSize[3];
    float hranges[2];
    const float* ranges[3];
    int channels[3];

  public:

    ColourHistogram() {

        // Prepare arguments for a colour histogram
        histSize[0]= histSize[1]= histSize[2]= 256;
        hranges[0]= 0.0;    // BRG range
        hranges[1]= 255.0;
        ranges[0]= hranges; // all channels have the same range
        ranges[1]= hranges;
        ranges[2]= hranges;
        channels[0]= 0;		// the three channels
        channels[1]= 1;
        channels[2]= 2;
    }

    // Computes the histogram.
    cv::MatND getHistogram(const cv::Mat &image) {

        cv::MatND hist;

        // BGR colour histogram
        hranges[0]= 0.0;    // BRG range
        hranges[1]= 255.0;
        channels[0]= 0;		// the three channels
        channels[1]= 1;
        channels[2]= 2;

        // Compute histogram
        cv::calcHist(&image,
            1,			// histogram of 1 image only
            channels,	// the channel used
            cv::Mat(),	// no mask is used
            hist,		// the resulting histogram
            3,			// it is a 3D histogram
            histSize,	// number of bins
            ranges		// pixel value range
        );

        return hist;
    }

    // Computes the histogram.
    cv::SparseMat getSparseHistogram(const cv::Mat &image) {

        cv::SparseMat hist(3,histSize,CV_32F);

        // BGR colour histogram
        hranges[0]= 0.0;    // BRG range
        hranges[1]= 255.0;
        channels[0]= 0;		// the three channels
        channels[1]= 1;
        channels[2]= 2;

        // Compute histogram
        cv::calcHist(&image,
            1,			// histogram of 1 image only
            channels,	// the channel used
            cv::Mat(),	// no mask is used
            hist,		// the resulting histogram
            3,			// it is a 3D histogram
            histSize,	// number of bins
            ranges		// pixel value range
        );

        return hist;
    }

    // Computes the 1D Hue histogram with a mask.
    // BGR source image is converted to HSV
    // Pixels with low saturation are ignored
    cv::MatND getHueHistogram(const cv::Mat &image,
                             int minSaturation=0) {

        cv::MatND hist;

        // Convert to HSV colour space
        cv::Mat hsv;
        cv::cvtColor(image, hsv, CV_BGR2HSV);

        // Mask to be used (or not)
        cv::Mat mask;

        if (minSaturation>0) {

            // Spliting the 3 channels into 3 images
            std::vector<cv::Mat> v;
            cv::split(hsv,v);

            // Mask out the low saturated pixels
            cv::threshold(v[1],mask,minSaturation,255,
                                 cv::THRESH_BINARY);
        }

        // Prepare arguments for a 1D hue histogram
        hranges[0]= 0.0;
        hranges[1]= 180.0;
        channels[0]= 0; // the hue channel

        // Compute histogram
        cv::calcHist(&hsv,
            1,			// histogram of 1 image only
            channels,	// the channel used
            mask,		// binary mask
            hist,		// the resulting histogram
            1,			// it is a 1D histogram
            histSize,	// number of bins
            ranges		// pixel value range
        );

        return hist;
    }

    // Computes the 1D Hue histogram with a mask.
    // BGR source image is converted to HSV
    cv::MatND getHueHistogram(const cv::Mat &image) {

        cv::MatND hist;

        // Convert to Lab colour space
        cv::Mat hue;
        cv::cvtColor(image, hue, CV_BGR2HSV);

        // Prepare arguments for a 1D hue histogram
        hranges[0]= 0.0;
        hranges[1]= 180.0;
        channels[0]= 0; // the hue channel

        // Compute histogram
        cv::calcHist(&hue,
            1,			// histogram of 1 image only
            channels,	// the channel used
            cv::Mat(),	// no mask is used
            hist,		// the resulting histogram
            1,			// it is a 1D histogram
            histSize,	// number of bins
            ranges		// pixel value range
        );

        return hist;
    }

    cv::Mat colorReduce(const cv::Mat &image, int div=64) {

      int n= static_cast<int>(log(static_cast<double>(div))/log(2.0));
      // mask used to round the pixel value
      uchar mask= 0xFF<<n; // e.g. for div=16, mask= 0xF0

      cv::Mat_<cv::Vec3b>::const_iterator it= image.begin<cv::Vec3b>();
      cv::Mat_<cv::Vec3b>::const_iterator itend= image.end<cv::Vec3b>();

      // Set output image (always 1-channel)
      cv::Mat result(image.rows,image.cols,image.type());
      cv::Mat_<cv::Vec3b>::iterator itr= result.begin<cv::Vec3b>();

      for ( ; it!= itend; ++it, ++itr) {

        (*itr)[0]= ((*it)[0]&mask) + div/2;
        (*itr)[1]= ((*it)[1]&mask) + div/2;
        (*itr)[2]= ((*it)[2]&mask) + div/2;
      }

      return result;
}

};


class ImageComparator {

  private:

    cv::Mat reference;
    cv::Mat input;
    cv::MatND refH;
    cv::MatND inputH;

    ColourHistogram hist;
    int div;

  public:

    ImageComparator() : div(32) {

    }

    // Color reduction factor
    // The comparaison will be made on images with
    // color space reduced by this factor in each dimension
    void setColorReduction( int factor) {

        div= factor;
    }

    int getColorReduction() {

        return div;
    }

    void setReferenceImage(const cv::Mat& image) {

        reference= hist.colorReduce(image,div);
        refH= hist.getHistogram(reference);
    }

    double compare(const cv::Mat& image) {

        input= hist.colorReduce(image,div);
        inputH= hist.getHistogram(input);

        return cv::compareHist(refH,inputH,CV_COMP_INTERSECT);
    }
};


#endif // IMAGEPROCESSING_H
