
#include "watchDetect.h"
#include "stdio.h"


CirclesDetecter::CirclesDetecter(){
    kernel = cvCreateStructuringElementEx(7, 3, 3, 1, CV_SHAPE_ELLIPSE);
    storage = cvCreateMemStorage(0);
    bitImage=NULL;
    contours=NULL;
    pattern=NULL;
    length=200;
    radious=60;
    patternIsNew=true;
    validArea=NULL;
}

CirclesDetecter::~CirclesDetecter(){
    cvReleaseStructuringElement(&kernel);
    if(bitImage)
        cvReleaseImage(&bitImage);
    if(pattern)
        cvReleaseImage(&pattern);
    if(validArea)
        free(validArea);
    cvReleaseMemStorage(&storage);
}


void CirclesDetecter::set_figure(int gray, float len, float r){
   // threshold=gray;
    length=len;
    radious=r;
}

int CirclesDetecter::detect(const IplImage *img){
    if(!bitImage)
        bitImage=cvCreateImage(cvGetSize(img),IPL_DEPTH_8U, 1);
    else if(bitImage->width!=img->width || bitImage->height!=img->height){
        cvReleaseImage(&bitImage);
        bitImage=cvCreateImage(cvGetSize(img),IPL_DEPTH_8U, 1);
    }

    cvThreshold(img, bitImage, threshold, 255, CV_THRESH_BINARY);
    cvMorphologyEx(bitImage, bitImage, NULL, kernel, CV_MOP_OPEN);
    cvClearMemStorage(storage);
    positions.clear();
    cvFindContours( bitImage, storage, &contours, sizeof(CvContour),
                        CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));
    CvSeq* c=contours;
    CvSeq* pre=NULL;
    CvSeq* tmpSeq=NULL;
    float ra, rb, cx, cy;
    double len;
    double maxLen= length*1.2, minLen= length*0.8;
    double maxR= radious*1.2, minR=radious*0.8;
    double maxX, minX, maxY, minY;
    if(validArea){
        minX=validArea->x;
        maxX=validArea->x+validArea->width;
        minY=validArea->y;
        maxY=validArea->y+validArea->height;
    }
    else{
        minX=-1; maxX=9999999;
        minY=-1; maxY=9999999;
    }

    bool need;
    while(c){
        CvBox2D ellipse;
        len= cvArcLength(c);
        need=false;
        if(len>minLen && len<maxLen){
            //printf("%f\n",len);
            ellipse = cvFitEllipse2(c);
            ra=ellipse.size.height;
            rb=ellipse.size.width;
            //printf("a=%f,b=%f\n", ra, rb);
            if(ra/rb<1.5 && rb>minR && ra<maxR){
                cx=ellipse.center.x;
                cy=ellipse.center.y;
                if(cx>minX && cx<maxX && cy>minY && cy<maxY){
                    need=true;
                    positions.push_back(Point(ellipse.center.x, ellipse.center.y));
                }
            }
        }

        if(need){
            pre=c;
            c=c->h_next;
        }
        else{
            tmpSeq=c->h_next;
            if(pre)
                pre->h_next= tmpSeq;
            else
                 contours=tmpSeq;
            cvSeqRemoveSlice(c, CV_WHOLE_SEQ);
            c=tmpSeq;
        }

    }
    return 0;
}

void CirclesDetecter::set_pattern(IplImage *img, const CvRect *area){
    if(img==NULL || img->nChannels!=1)
        return;
    if(pattern)
        cvReleaseImage(&pattern);
    if(area==NULL){
        pattern=cvCloneImage(img);
    }
    else{
        pattern=cvCreateImage(cvSize(area->width, area->height), IPL_DEPTH_8U, 1);
        cvSetImageROI(img, *area);
        cvCopy(img, pattern);
        cvResetImageROI(img);
        learn_pattern();
        patternIsNew=true;
    }
}

bool CirclesDetecter::pattern_is_new(){
    bool tmp=patternIsNew;
    patternIsNew=false;
    return tmp;
}

void CirclesDetecter::learn_pattern(){
    if(!pattern)
        return;
    IplImage* bw=cvCreateImage(cvGetSize(pattern), IPL_DEPTH_8U, 1);
    cvThreshold(pattern, bw, threshold, 255, CV_THRESH_BINARY);
    CvSeq *contr=0, *c=0, *cc=0;
    cvFindContours(bw,storage, &contr, sizeof(CvContour),
                    CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));
    c=contr;
    float len, maxLen=0;
    while(c){
        len= cvArcLength(c);
        if(len>maxLen){
            maxLen=len;
            cc=c;
        }
        c=c->h_next;
    }
    if(cc){
        length=maxLen;
        CvBox2D ellipse = cvFitEllipse2(cc);
        radious= (ellipse.size.height+ellipse.size.width)/2;
        printf("pattern learn: lenth=%f, r=%f\n", length, radious);
    }
}

void CirclesDetecter::set_area(CvRect *area){
    if(!area){
        if(validArea){
            free(validArea);
            validArea=NULL;
        }
    }
    else{
        if(!validArea)
            validArea= (CvRect*)malloc(sizeof(CvRect));
        *validArea=*area;
    }
}

WatchCircleDetecter::WatchCircleDetecter()
    :m_patternIsNew(false),m_patternImage(NULL),m_pattern(NULL),
      m_roiImage(NULL),m_convolution(NULL),m_roi(cvRect(0, 0, 0, 0)){
}

WatchCircleDetecter::~WatchCircleDetecter(){
    if(m_patternImage)
        cvReleaseImage(&m_patternImage);
    if(m_pattern)
        cvReleaseMat(&m_pattern);
    if(m_roiImage)
        cvReleaseImage(&m_roiImage);
    if(m_convolution)
        cvReleaseImage(&m_convolution);
}

void WatchCircleDetecter::set_pattern(IplImage *img, const CvRect *area){
    if(img==NULL || img->nChannels!=1)
        return;
    if(m_patternImage)
        cvReleaseImage(&m_patternImage);
    if(area==NULL){
        m_patternImage=cvCloneImage(img);
    }
    else{
        m_patternImage=cvCreateImage(cvSize(area->width, area->height), IPL_DEPTH_8U, 1);
        cvSetImageROI(img, *area);
        cvCopy(img, m_patternImage);
        cvResetImageROI(img);
    }
    m_patternIsNew=true;
    setCirclePattern(m_patternImage, m_patternImage->width/2-10);
}

void WatchCircleDetecter::setCirclePattern(const IplImage *pattern, float R, int outWidth, float likelihood){
    assert(R>1 && outWidth>0);
    m_R=R;
    int halfwidth=R+outWidth+0.5;
    int width=2*halfwidth+1;
    int inWidth=R/2;
    if(inWidth>5)
        inWidth=5;
    else if(inWidth<1)
        inWidth=1;

    //create ideal pattern
    if(m_pattern)
        cvReleaseMat(&m_pattern);
    m_pattern=cvCreateMat(width, width, CV_32FC1);
    float d2;
    float R2=R*R;
    float OR2=(R+outWidth)*(R+outWidth);
    float IR2=(R-inWidth)*(R-inWidth);
    float val;
    for(int row=0;row<width;row++){
        for(int col=0;col<width; col++){
            d2=(row-halfwidth)*(row-halfwidth)+(col-halfwidth)*(col-halfwidth);
            if(d2<IR2)
                val=0;
            else if(d2<R2)
                val=1;
            else if(d2<OR2)
                val=-inWidth/outWidth;
            else
                val=0;
            cvSetReal2D(m_pattern,row, col, val);
        }
    }

    //get the threshold from actual pattern
    IplImage* smooth=cvCreateImage(cvGetSize(pattern),IPL_DEPTH_8U, 1);
    IplImage* convolution=cvCreateImage(cvGetSize(smooth),IPL_DEPTH_32F, 1);
    cvSmooth(pattern,smooth, CV_GAUSSIAN);
    cvFilter2D(smooth, convolution, m_pattern);
    double minVal, maxVal;
    cvMinMaxLoc(convolution, &minVal,&maxVal);
    m_threshold=maxVal*likelihood;
    cvReleaseImage(&smooth);
    cvReleaseImage(&convolution);
}


const list<Point>& WatchCircleDetecter::detect(IplImage *image, CvRect *roi){
    assert(m_pattern);
    m_centers.clear();
    vector<double> weightSum;
    vector<double> xWeight;
    vector<double> yWeight;

    //copy ROI
    if(roi){
        if(roi->x<0 ||roi->y<0 ||roi->width<m_pattern->width || roi->height<m_pattern->height
                || roi->x+roi->width>image->width || roi->y+roi->height>image->height)
        {
            printf("intersting area is error\n");
            return m_centers;
        }
        m_roi=*roi;
    }
    else
        m_roi=cvRect(0, 0, image->width, image->height);
    if(m_roiImage==NULL){
        m_roiImage=cvCreateImage(cvSize(m_roi.width, m_roi.height), IPL_DEPTH_8U, 1);
        m_convolution=cvCreateImage(cvGetSize(m_roiImage),IPL_DEPTH_32F, 1);
    }
    else{
        if(m_roiImage->width!=roi->width || m_roiImage->height!=roi->height){
            cvReleaseImage(&m_roiImage);
            cvReleaseImage(&m_convolution);
            m_roiImage=cvCreateImage(cvSize(m_roi.width, m_roi.height), IPL_DEPTH_8U, 1);
            m_convolution=cvCreateImage(cvGetSize(m_roiImage),IPL_DEPTH_32F, 1);
        }
    }
    cvSetImageROI(image, m_roi);
    cvCopyImage(image, m_roiImage);
    cvResetImageROI(image);

    //convolution
    cvSmooth(m_convolution, m_convolution,CV_GAUSSIAN);
    cvFilter2D(m_roiImage, m_convolution, m_pattern);

    //find center
    char* rowHead=m_convolution->imageData;
    float* p=NULL;
    float maxDistance=0.5*m_R*0.5*m_R;
    list<Point>::iterator it;
    for(int row=0; row<m_convolution->height; row++){
        p=(float*)rowHead;
        for(int col=0;col<m_convolution->width;col++){
            if(*p>m_threshold){
                bool newCenter=true;
                int index=0;
                for(it=m_centers.begin();it!=m_centers.end();it++,index++){
                    int dx=it->x()-col;
                    int dy=it->y()-row;
                    if(dx*dx+dy*dy < maxDistance){
                        newCenter=false;
                        xWeight[index]+=*p*col;
                        yWeight[index]+=*p*row;
                        weightSum[index]+=*p;
                        double x=xWeight[index]/weightSum[index];
                        double y=yWeight[index]/weightSum[index];
                        it->set(x,y);
                    }
                }
                if(newCenter){
                    m_centers.push_back(Point(col, row));
                    xWeight.push_back(*p*col);
                    yWeight.push_back(*p*row);
                    weightSum.push_back(*p);
                }
            }
            p++;
        }
        rowHead+=m_convolution->widthStep;
    }

    //only retain whole circle
    Vector2 vct(m_roi.x, m_roi.y);
    for(it=m_centers.begin();it!=m_centers.end();){
        if(it->x()<m_R || it->y()<m_R || it->x()>m_convolution->width-m_R || it->y()>m_convolution->height-m_R){
            list<Point>::iterator rmIt=it;
            it++;
            m_centers.erase(rmIt);
        }
        else{
            it->move(vct);
            it++;
        }
    }

    return m_centers;
}

bool WatchCircleDetecter::pattern_is_new(){
    bool tmp=m_patternIsNew;
    m_patternIsNew=false;
    return tmp;
}
