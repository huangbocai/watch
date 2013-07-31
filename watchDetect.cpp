
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
    :m_patternIsNew(false),m_patternImage(NULL),level(0),
     m_roi(cvRect(0, 0, 0, 0)){
    for(int i=0;i<maxLevel;i++){
        m_pattern[i]=NULL;
        m_roiImage[i]=NULL;
        m_convolution[i]=NULL;
        m_blockImage[i]=NULL;
    }
}

WatchCircleDetecter::~WatchCircleDetecter(){
    if(m_patternImage)
        cvReleaseImage(&m_patternImage);
    for(int i=0;i<maxLevel;i++){
        if(m_pattern[i])
            cvReleaseMat(&m_pattern[i]);
        if(m_roiImage[i])
            cvReleaseImage(&m_roiImage[i]);
        if(m_convolution[i])
            cvReleaseImage(&m_convolution[i]);
        if(m_blockImage[i])
            cvReleaseImage(&m_blockImage[i]);
    }
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

CvMat* WatchCircleDetecter::createIdealPattern(float R, float outWidth){
    assert(R>1 && outWidth>0);
    int width=2*(outWidth+R)+0.5;
    float center=(width-1)/2.0;
    int inWidth=R/2;
    if(inWidth>5)
        inWidth=5;
    else if(inWidth<1)
        inWidth=1;
    CvMat* pattern=cvCreateMat(width, width, CV_32FC1);
    float d2;
    float R2=R*R;
    float OR2=(R+outWidth)*(R+outWidth);
    float IR2=(R-inWidth)*(R-inWidth);
    float val;
    for(int row=0;row<width;row++){
        for(int col=0;col<width; col++){
            d2=(row-center)*(row-center)+(col-center)*(col-center);
            if(d2<IR2)
                val=0;
            else if(d2<R2)
                val=1;
            else if(d2<OR2)
                val=-1.0*inWidth/outWidth;
            else
                val=0;
            cvSetReal2D(pattern,row, col, val);
        }
    }
    return pattern;
}

void WatchCircleDetecter::setCirclePattern(const IplImage *pattern, float R, float outWidth, float likelihood){
    assert(R>1 && outWidth>0);
    m_R=R;

    //create ideal patterns
    for(int i=0;i<=level;i++){
        if(m_pattern[i])
            cvReleaseMat(&m_pattern[i]);
    }
    level=0;
    m_pattern[0]=createIdealPattern(R, outWidth);
    while(R>8){
        R/=2;
        outWidth/=2;
        if(outWidth<1)
            outWidth=1;
        level++;
        m_pattern[level]=createIdealPattern(R,outWidth);
    }


    //get the threshold from actual pattern
    IplImage* src=cvCreateImage(cvGetSize(pattern),IPL_DEPTH_8U, 1);
    cvSmooth(pattern,src, CV_GAUSSIAN);
    IplImage* dst=NULL;
    for(int i=0;i<level;i++){
        dst=cvCreateImage(cvSize(src->width/2, src->height/2),IPL_DEPTH_8U, 1);
        cvPyrDown(src, dst);
        cvReleaseImage(&src);
        src=dst;
        dst=NULL;
    }

    IplImage* convolution=cvCreateImage(cvGetSize(src),IPL_DEPTH_32F, 1);
    cvFilter2D(src, convolution, m_pattern[level], cvPoint(0, 0));
    double minVal, maxVal;
    cvMinMaxLoc(convolution, &minVal,&maxVal);
    m_threshold=maxVal*likelihood;
    cvReleaseImage(&src);
    cvReleaseImage(&convolution);
}

const list<Point>& WatchCircleDetecter::detect(IplImage *image, CvRect *roi){

    //inital
    m_centers.clear();

    if(!m_pattern[0])
        return m_centers;

    //copy ROI
    if(roi){
        if(roi->width<=0 || roi->height<=0 || roi->x<0 || roi->y<0
                || roi->x+roi->width>image->width || roi->y+roi->height>image->height)
            return m_centers;
        m_roi=*roi;
    }
    else
        m_roi=cvRect(0, 0, image->width, image->height);

    //create ROI images
    if(m_roiImage[0]==NULL){
        m_roiImage[0]=cvCreateImage(cvSize(m_roi.width, m_roi.height), IPL_DEPTH_8U, 1);
    }
    else if(m_roiImage[0]->width!=roi->width || m_roiImage[0]->height!=roi->height){
        cvReleaseImage(&m_roiImage[0]);
        m_roiImage[0]=cvCreateImage(cvSize(m_roi.width, m_roi.height), IPL_DEPTH_8U, 1);
    }
    cvSetImageROI(image, m_roi);
    cvCopyImage(image, m_roiImage[0]);
    cvResetImageROI(image);
    cvSmooth(m_roiImage[0], m_roiImage[0],CV_GAUSSIAN);

    for(int i=1;i<=level;i++){
        if(m_roiImage[i]==NULL)
            m_roiImage[i]=cvCreateImage(cvSize(m_roiImage[i-1]->width/2, m_roiImage[i-1]->height/2), IPL_DEPTH_8U, 1);
        else if(m_roiImage[i]->width!=m_roiImage[i-1]->width/2
                || m_roiImage[i]->height!=m_roiImage[i-1]->height/2){
            cvReleaseImage(&m_roiImage[i]);
        m_roiImage[i]=cvCreateImage(cvSize(m_roiImage[i-1]->width/2, m_roiImage[i-1]->height/2), IPL_DEPTH_8U, 1);
        }
        cvPyrDown(m_roiImage[i-1], m_roiImage[i]);
    }


    //convolution the min image
    if(m_convolution[level]==NULL){
        m_convolution[level]=cvCreateImage(cvGetSize(m_roiImage[level]), IPL_DEPTH_32F, 1);
    }
    cvFilter2D(m_roiImage[level], m_convolution[level], m_pattern[level], cvPoint(0,0));


    //find centers in min image
    vector<float> maxVals;
    char* rowHead=m_convolution[level]->imageData;
    float* p=NULL;
    float r=m_R*pow(0.5, level);
    float maxDistance=r*r;
    int imgWidth=m_convolution[level]->width;
    int imgHeight=m_convolution[level]->height;
    int widthStep=m_convolution[level]->widthStep;
    list<Point>::iterator it;

    for(int row=0; row<imgHeight; row++){
        p=(float*)rowHead;
        for(int col=0;col<imgWidth;col++){
            if(*p>m_threshold){
                bool newCenter=true;
                int index=0;
                for(it=m_centers.begin();it!=m_centers.end();it++,index++){
                    int dx=it->x()-col+0.5;
                    int dy=it->y()-row+0.5;
                    if(dx*dx+dy*dy < maxDistance){
                        newCenter=false;
                        if(*p>maxVals[index]){
                            maxVals[index]=*p;
                            it->set(col,row);
                        }
                    }
                }
                if(newCenter){
                    m_centers.push_back(Point(col, row));
                    maxVals.push_back(*p);
                }
            }
            p++;
        }
        rowHead+=widthStep;
    }

    //remove the partial circle, only retain whole circle
    for(it=m_centers.begin();it!=m_centers.end();){
        if(it->x() > m_convolution[level]->width-m_pattern[level]->width
                ||it->y()> m_convolution[level]->height-m_pattern[level]->height){
            list<Point>::iterator rmIt=it;
            it++;
            m_centers.erase(rmIt);
        }
        else
            it++;
    }

  #ifdef debug_algorithm
    IplImage* rgb=cvCreateImage(cvGetSize(m_roiImage[level]), IPL_DEPTH_8U, 3);
    cvCvtColor(m_roiImage[level], rgb, CV_GRAY2RGB);
    for(it=m_centers.begin();it!=m_centers.end();it++){
        int cx=it->x+(m_pattern[level]->width-1)/2;
        int cy=it->y+(m_pattern[level]->height-1)/2;
        cvDrawLine(rgb, cvPoint(cx,cy-5), cvPoint(cx, cy+5),cvScalar(0,0,255));
        cvDrawLine(rgb, cvPoint(cx-5,cy), cvPoint(cx+5, cy),cvScalar(0,0,255));
    }
    cvSaveImage("min.jpg", rgb);
 #endif

    //toward top level
    CvRect blockRoi;
    for(int l=level-1;l>=0;l--){
        blockRoi.width=m_pattern[l]->width+blockRange;
        blockRoi.height=m_pattern[l]->height+blockRange;
        if(blockRoi.width>m_roiImage[l]->width)
            blockRoi.width=m_roiImage[l]->width;
        if(blockRoi.height>m_roiImage[l]->height)
            blockRoi.height=m_roiImage[l]->height;
        if(m_blockImage[l]==NULL){
            m_blockImage[l]=cvCreateImage(cvSize(blockRoi.width, blockRoi.height), IPL_DEPTH_8U, 1);
            m_convolution[l]=cvCreateImage(cvGetSize(m_blockImage[l]), IPL_DEPTH_32F, 1);
        }
        else if(m_blockImage[l]->width!=blockRoi.width || m_blockImage[l]->height!=blockRoi.height){
            cvReleaseImage(&m_blockImage[l]);
            cvReleaseImage(&m_convolution[l]);
            m_blockImage[l]=cvCreateImage(cvSize(blockRoi.width, blockRoi.height), IPL_DEPTH_8U, 1);
            m_convolution[l]=cvCreateImage(cvGetSize(m_blockImage[l]), IPL_DEPTH_32F, 1);
        }
        for(it=m_centers.begin();it!=m_centers.end();it++){
            blockRoi.x=it->x()*2-(blockRange-1)/2+0.5;
            blockRoi.y=it->y()*2-(blockRange-1)/2+0.5;
            if(blockRoi.x<0)
                blockRoi.x=0;
            if(blockRoi.y<0)
                blockRoi.y=0;
            if(blockRoi.x+blockRoi.width>m_roiImage[l]->width)
                blockRoi.x=m_roiImage[l]->width-blockRoi.width;
            if(blockRoi.y+blockRoi.height>m_roiImage[l]->height)
                blockRoi.y=m_roiImage[l]->height-blockRoi.height;
            cvSetImageROI(m_roiImage[l], blockRoi);
            cvCopy(m_roiImage[l], m_blockImage[l]);
            cvResetImageROI(m_roiImage[l]);
            cvFilter2D(m_blockImage[l],m_convolution[l],m_pattern[l],cvPoint(0,0));
            //find max
            rowHead=m_convolution[l]->imageData;
            float maxVal=0;
            for(int row=0;row<=blockRange;row++){
                p=(float*)rowHead;
                for(int col=0;col<=blockRange;col++){
                    if(*p>maxVal){
                        maxVal=*p;
                        it->set(blockRoi.x+col, blockRoi.y+row);
                    }
                    p++;
                }
                rowHead+=m_convolution[l]->widthStep;
            }

        }
    }

    for(it=m_centers.begin();it!=m_centers.end();it++){
        it->set(it->x()+(m_pattern[0]->width-1)/2.0, it->y()+(m_pattern[0]->height-1)/2.0);
    }

    return m_centers;
}




bool WatchCircleDetecter::pattern_is_new(){
    bool tmp=m_patternIsNew;
    m_patternIsNew=false;
    return tmp;
}
