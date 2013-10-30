
#include "watchDetect.h"
#include "stdio.h"
#include "highgui.h"
#include <set>

//using namespace cv;

//CirclesDetecter::CirclesDetecter(){
//    kernel = cvCreateStructuringElementEx(7, 3, 3, 1, CV_SHAPE_ELLIPSE);
//    storage = cvCreateMemStorage(0);
//    bitImage=NULL;
//    contours=NULL;
//    pattern=NULL;
//    length=200;
//    radious=60;
//    patternIsNew=true;
//    validArea=NULL;
//}

//CirclesDetecter::~CirclesDetecter(){
//    cvReleaseStructuringElement(&kernel);
//    if(bitImage)
//        cvReleaseImage(&bitImage);
//    if(pattern)
//        cvReleaseImage(&pattern);
//    if(validArea)
//        free(validArea);
//    cvReleaseMemStorage(&storage);
//}


//void CirclesDetecter::set_figure(int gray, float len, float r){
//   // threshold=gray;
//    length=len;
//    radious=r;
//}

//int CirclesDetecter::detect(const IplImage *img){
//    if(!bitImage)
//        bitImage=cvCreateImage(cvGetSize(img),IPL_DEPTH_8U, 1);
//    else if(bitImage->width!=img->width || bitImage->height!=img->height){
//        cvReleaseImage(&bitImage);
//        bitImage=cvCreateImage(cvGetSize(img),IPL_DEPTH_8U, 1);
//    }

//    cvThreshold(img, bitImage, threshold, 255, CV_THRESH_BINARY);
//    cvMorphologyEx(bitImage, bitImage, NULL, kernel, CV_MOP_OPEN);
//    cvClearMemStorage(storage);
//    positions.clear();
//    cvFindContours( bitImage, storage, &contours, sizeof(CvContour),
//                        CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));
//    CvSeq* c=contours;
//    CvSeq* pre=NULL;
//    CvSeq* tmpSeq=NULL;
//    float ra, rb, cx, cy;
//    double len;
//    double maxLen= length*1.2, minLen= length*0.8;
//    double maxR= radious*1.2, minR=radious*0.8;
//    double maxX, minX, maxY, minY;
//    if(validArea){
//        minX=validArea->x;
//        maxX=validArea->x+validArea->width;
//        minY=validArea->y;
//        maxY=validArea->y+validArea->height;
//    }
//    else{
//        minX=-1; maxX=9999999;
//        minY=-1; maxY=9999999;
//    }

//    bool need;
//    while(c){
//        CvBox2D ellipse;
//        len= cvArcLength(c);
//        need=false;
//        if(len>minLen && len<maxLen){
//            //printf("%f\n",len);
//            ellipse = cvFitEllipse2(c);
//            ra=ellipse.size.height;
//            rb=ellipse.size.width;
//            //printf("a=%f,b=%f\n", ra, rb);
//            if(ra/rb<1.5 && rb>minR && ra<maxR){
//                cx=ellipse.center.x;
//                cy=ellipse.center.y;
//                if(cx>minX && cx<maxX && cy>minY && cy<maxY){
//                    need=true;
//                    positions.push_back(Point(ellipse.center.x, ellipse.center.y));
//                }
//            }
//        }

//        if(need){
//            pre=c;
//            c=c->h_next;
//        }
//        else{
//            tmpSeq=c->h_next;
//            if(pre)
//                pre->h_next= tmpSeq;
//            else
//                 contours=tmpSeq;
//            cvSeqRemoveSlice(c, CV_WHOLE_SEQ);
//            c=tmpSeq;
//        }

//    }
//    return 0;
//}

//void CirclesDetecter::set_pattern(IplImage *img, const CvRect *area){
//    if(img==NULL || img->nChannels!=1)
//        return;
//    if(pattern)
//        cvReleaseImage(&pattern);
//    if(area==NULL){
//        pattern=cvCloneImage(img);
//    }
//    else{
//        pattern=cvCreateImage(cvSize(area->width, area->height), IPL_DEPTH_8U, 1);
//        cvSetImageROI(img, *area);
//        cvCopy(img, pattern);
//        cvResetImageROI(img);
//        learn_pattern();
//        patternIsNew=true;
//    }
//}

//bool CirclesDetecter::pattern_is_new(){
//    bool tmp=patternIsNew;
//    patternIsNew=false;
//    return tmp;
//}

//void CirclesDetecter::learn_pattern(){
//    if(!pattern)
//        return;
//    IplImage* bw=cvCreateImage(cvGetSize(pattern), IPL_DEPTH_8U, 1);
//    cvThreshold(pattern, bw, threshold, 255, CV_THRESH_BINARY);
//    CvSeq *contr=0, *c=0, *cc=0;
//    cvFindContours(bw,storage, &contr, sizeof(CvContour),
//                    CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));
//    c=contr;
//    float len, maxLen=0;
//    while(c){
//        len= cvArcLength(c);
//        if(len>maxLen){
//            maxLen=len;
//            cc=c;
//        }
//        c=c->h_next;
//    }
//    if(cc){
//        length=maxLen;
//        CvBox2D ellipse = cvFitEllipse2(cc);
//        radious= (ellipse.size.height+ellipse.size.width)/2;
//        printf("pattern learn: lenth=%f, r=%f\n", length, radious);
//    }
//}

//void CirclesDetecter::set_area(CvRect *area){
//    if(!area){
//        if(validArea){
//            free(validArea);
//            validArea=NULL;
//        }
//    }
//    else{
//        if(!validArea)
//            validArea= (CvRect*)malloc(sizeof(CvRect));
//        *validArea=*area;
//    }
//}

WatchCircleDetecter::WatchCircleDetecter()
    :m_roi(cvRect(0, 0, 0, 0)),m_patternIsNew(false),m_patternImage(NULL),
      level(0),m_threshold(0.5),m_topLevel(0){
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
            cvReleaseImage(&m_pattern[i]);
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

IplImage* WatchCircleDetecter::createIdealPattern(float R, float outWidth){
    assert(R>1 && outWidth>0);
    int width=2*(outWidth+R)+0.5;
    float center=(width-1)/2.0;
    IplImage* pattern=cvCreateImage(cvSize(width, width), IPL_DEPTH_8U, 1);
    float d2;
    float RR=R*R;
    float OR2=(R+outWidth)*(R+outWidth);
    char *p, *rowHead=pattern->imageData;
    for(int row=0;row<width;row++){
        p=rowHead;
        for(int col=0;col<width; col++){
            d2=(row-center)*(row-center)+(col-center)*(col-center);
            if(d2<RR)
                *p=255;
            else if(d2<OR2)
                *p=0;
            else
                *p=128;
            p++;
        }
        rowHead+=pattern->widthStep;
    }
#ifdef debug_algorithm
//    static int n=0;
//    char buf[256];
//    sprintf(buf, "pattern%d.ppm", n++);
//    cvSaveImage(buf, pattern);
#endif
    return pattern;
}

void WatchCircleDetecter::setCirclePattern(const IplImage *pattern, float R, float outWidth){
    assert(R>1 && outWidth>0);
    m_R=R;

    //create ideal patterns
    for(int i=0;i<=level;i++){
        if(m_pattern[i])
            cvReleaseImage(&m_pattern[i]);
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
}



const list<hbc::Point>& WatchCircleDetecter::detect(IplImage *image, CvRect *roi){

    //printf("watch detect\n");
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

    //double t = cv::getTickCount();
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
    int conW=m_roiImage[level]->width-m_pattern[level]->width+1;
    int conH=m_roiImage[level]->height-m_pattern[level]->height+1;
    if(m_convolution[level]==NULL){
        m_convolution[level]=cvCreateImage(cvSize(conW, conH), IPL_DEPTH_32F, 1);
    }
    else if(m_convolution[level]->width!=conW || m_convolution[level]->height!=conH){
        cvReleaseImage(&m_convolution[level]);
         m_convolution[level]=cvCreateImage(cvSize(conW, conH), IPL_DEPTH_32F, 1);
    }
    cvMatchTemplate(m_roiImage[level], m_pattern[level], m_convolution[level],CV_TM_CCOEFF_NORMED);


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
                    float dx=it->x()-col;
                    float dy=it->y()-row;
                    if(dx*dx+dy*dy < maxDistance){
                        newCenter=false;
                        if(*p>maxVals[index]){
                            maxVals[index]=*p;
                            it->set(col, row);
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
        if(it->x()<1 || it->y()<1 || it->x() > m_convolution[level]->width-2
                ||it->y() > m_convolution[level]->height-2){
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
    int l;
    for(l=level-1;l>=m_topLevel;l--){
        blockRoi.width=m_pattern[l]->width+blockRange;
        blockRoi.height=m_pattern[l]->height+blockRange;
        if(blockRoi.width>m_roiImage[l]->width)
            blockRoi.width=m_roiImage[l]->width;
        if(blockRoi.height>m_roiImage[l]->height)
            blockRoi.height=m_roiImage[l]->height;
        if(m_blockImage[l]==NULL){
            m_blockImage[l]=cvCreateImage(cvSize(blockRoi.width, blockRoi.height), IPL_DEPTH_8U, 1);
            m_convolution[l]=cvCreateImage(cvSize(blockRange+1, blockRange+1), IPL_DEPTH_32F, 1);
        }
        else if(m_blockImage[l]->width!=blockRoi.width || m_blockImage[l]->height!=blockRoi.height){
            cvReleaseImage(&m_blockImage[l]);
//            cvReleaseImage(&m_convolution[l]);
            m_blockImage[l]=cvCreateImage(cvSize(blockRoi.width, blockRoi.height), IPL_DEPTH_8U, 1);
//            m_convolution[l]=cvCreateImage(cvGetSize(m_blockImage[l]), IPL_DEPTH_32F, 1);
        }
        for(it=m_centers.begin();it!=m_centers.end();){
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

//            cvFilter2D(m_blockImage[l],m_convolution[l],m_pattern[l],cvPoint(0,0));
            cvMatchTemplate(m_blockImage[l], m_pattern[l], m_convolution[l], CV_TM_CCOEFF_NORMED);
            //find max
            double minVal, maxVal;
            CvPoint pos;
            cvMinMaxLoc(m_convolution[l],&minVal, &maxVal, NULL, &pos);
//            if(maxVal>m_threshold){
            it->set(pos.x+blockRoi.x, pos.y+blockRoi.y);
//            }
//            else{
//                list<CvPoint>::iterator rmIt=it;
//                it++;
//                m_centers.erase(rmIt);
//                continue;
//            }
            it++;

        }
    }
    l++;

    for(it=m_centers.begin();it!=m_centers.end();it++){
        float x=it->x()*pow(2,l)+(m_pattern[0]->width-1)/2.0+m_roi.x;
        float y=it->y()*pow(2,l)+(m_pattern[0]->height-1)/2.0+m_roi.y;
        it->set(x,y);
    }

    //double tt =((double)cv::getTickCount()-t)/cv::getTickFrequency();
    //printf("Time: %.3f\n",tt);

    return m_centers;
}




bool WatchCircleDetecter::pattern_is_new(){
    bool tmp=m_patternIsNew;
    m_patternIsNew=false;
    return tmp;
}

DiamondCircleDetecter::DiamondCircleDetecter()
    :WatchCircleDetecter(),mPatternFgPixNum(0),mDtThreshold(60),
      mDtPixNumDiffer(1000),mDtSearchRegionWidth(130),mType(HOUGH_CIRCLE)
{
    finder = new CirclesFinder();
    setMinDist(20);
    setCannyHighThreshold(150);
    setMinVotes(70);
    setMinRadius(50);
    setMaxRadius(60);

}

DiamondCircleDetecter::~DiamondCircleDetecter()
{
    if(finder)
        delete finder;
}

IplImage* DiamondCircleDetecter::createIdealPattern(float R, float outWidth){
    outWidth=10;
    assert(R>1 && outWidth>0);
    int width=2*(outWidth+R)+0.5;
    float center=(width-1)/2.0;
    IplImage* pattern=cvCreateImage(cvSize(width, width), IPL_DEPTH_8U, 1);
    float d2;
    float RR=R*R;
    float OR2=(R+outWidth)*(R+outWidth);
    char *p, *rowHead=pattern->imageData;
    for(int row=0;row<width;row++){
        p=rowHead;
        for(int col=0;col<width; col++){
            d2=(row-center)*(row-center)+(col-center)*(col-center);
            if(d2<RR)
                *p=255;
            else
                *p=0;
            p++;
        }
        rowHead+=pattern->widthStep;
    }
#ifdef debug_algorithm
//    static int n=0;
//    char buf[256];
//    sprintf(buf, "pattern%d.ppm", n++);
//    cvSaveImage(buf, pattern);
#endif
    return pattern;
}


bool comp(const cv::Vec3i& v1, const cv::Vec3i& v2)
{
    return v1[0]<v2[0]?true:false;
}

bool compare_point(const Point& p1, const Point& p2)
{
    return p1.y()<p2.y()?true:false;

}

vector<cv::Mat> doPyrDown(cv::Mat srcImg)
{
    cv::Mat tmpMat;
    vector<cv::Mat> pyrDownImg;
    pyrDownImg.push_back(srcImg);
    for(int i=1; i<=1; i++){
        pyrDown(pyrDownImg[i-1],tmpMat);
        pyrDownImg.push_back(tmpMat);
    }
    return pyrDownImg;
}

const list<Point>& DiamondCircleDetecter::detect(IplImage* image, CvRect* roi)
{
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

    if(mType == DT){
        //printf("DT\n");
        return dtTransform(image);
    }
    else{
        //printf("Hough\n");
        return houghCircle(image);
    }
}

const list<Point>& DiamondCircleDetecter::dtTransform(IplImage *image)
{
    cv::Mat src(image);
    cv::Mat templateMat(m_pattern[0]);
    if(!src.data || !templateMat.data)
        return m_centers;
    //copy roi
    //cv::Mat src_roi(src,cv::Rect(m_roi));
    cv::Mat src_roi(src);

    double t = cv::getTickCount();
    vector<cv::Mat> srcDownVec = doPyrDown(src_roi);
    vector<cv::Mat> templateVec = doPyrDown(templateMat);
    cv::Mat src1 = srcDownVec.back();
    cv::Mat templateMat1 = templateVec.back();


    //binary
    cv::Mat bw;
    //cvtColor(src1,bw,CV_BGR2GRAY);
    threshold(src1,bw,mDtThreshold,255,CV_THRESH_BINARY);

    //do distance transform and get the brightest points
    cv::Mat dist;
    distanceTransform(bw,dist,CV_DIST_L2,3);
    normalize(dist,dist,0, 1., cv::NORM_MINMAX);
    threshold(dist,dist,0.6,1,CV_THRESH_BINARY);

    //find contours
    cv::Mat dist_8u;
    dist.convertTo(dist_8u,CV_8U);
    vector<vector<cv::Point> > contours;
    findContours(dist_8u,contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    int ncomp = contours.size();

    //make marker, which is needed in watershed
    cv::Mat markers = cv::Mat::zeros(dist.size(), CV_32SC1);
    for(int i = 0; i < ncomp; i++)
        drawContours(markers, contours, i, cv::Scalar::all(i+1), -1);
    circle(markers,cv::Point(5,5), 3, CV_RGB(255,255,255), -1);

    cv::Mat src_8uc3;
    cvtColor(src1,src_8uc3,CV_GRAY2BGR);

    cv::watershed(src_8uc3,markers);

    vector<cv::Vec3i> mark_points;
    for(int i = 0; i < markers.rows; i++)
    {
        for(int j = 0; j < markers.cols; j++)
        {
            int index = markers.at<int>(i,j);
            if(index >0 && index <= ncomp){
                mark_points.push_back(cv::Vec3i(index,j,i));
            }
        }
    }

    sort(mark_points.begin(),mark_points.end(),comp);

    vector<int> objAreaX;
    vector<int> objAreaY;
    vector<vector<int> > vec_x;
    vector<vector<int> > vec_y;

    int index=1;
    for(unsigned int i = 0; i < mark_points.size(); i++)
    {
        cv::Vec3i p = mark_points[i];
        if(p[0]<=ncomp){
            if(p[0] != index || i == mark_points.size()-1)
            {
                //cout<<"index: "<<p[0]<<endl;
                index++;
                vec_x.push_back(objAreaX);
                vec_y.push_back(objAreaY);
                objAreaX.clear();
                objAreaY.clear();
            }
            if(p[1]<templateMat1.cols/2 || p[2]<templateMat1.rows/2 || p[1]>src1.cols-2 || p[2]>src1.rows-2 )
                continue;
            objAreaX.push_back(p[1]);
            objAreaY.push_back(p[2]);
        }

    }

    int tw = templateMat1.cols-20/2;
    int th = templateMat1.rows-20/2;

    vector<hbc::Point> centers;
    for(unsigned int i=0; i<vec_x.size(); i++)
    {
        double minX, maxX, minY, maxY;
        cv::minMaxIdx(vec_x[i],&minX,&maxX);
        cv::minMaxIdx(vec_y[i],&minY,&maxY);
        int roiWidth =maxX-minX;
        int roiHeight =maxY-minY;

        if(roiWidth<tw-10 || roiWidth>tw+10 || roiHeight<th-10 || roiHeight>th+10)
            continue;


        cv::Rect bounding(minX-5,minY-5,roiWidth+10,roiHeight+10);
        cv::Point tl = bounding.tl();
        cv::Point br = bounding.br();
        hbc::Point center = hbc::Point((tl.x+br.x)/2,(tl.y+br.y)/2);
        centers.push_back(center);
    }

//    //delete points too close to each other
//    if(centers.size()>1){
//        sort(centers.begin(),centers.end(),compare_point);
//        vector<hbc::Point>::iterator it;
//        it = centers.begin();
//        Point p1 = *it;
//        it++;
//        for( ; it!=centers.end();){
//            Point p2 = *it;
//            if(abs(p1.y()-p2.y())<10 && abs(p1.x()-p2.x())<templateMat.cols+10){
//                it = centers.erase(it);
//                continue;
//            }
//            p1 = p2;
//            it++;
//        }
//    }

//    double minDis = templateMat1.cols;
//    set<int> closeDiamondSet;
//    if(centers.size()>1){
//        unsigned int i=0, j=0;
//        Point p1,p2;
//        double dis;
//        for(i=0 ; i<centers.size(); i++){
//            p1 = centers[i];
//            for(j=0 ; j<centers.size(); j++){
//                if(j == i || closeDiamondSet.find(j)!=closeDiamondSet.end())
//                    continue;
//                p2 = centers[j];
//                Vector2 vec(p1,p2);
//                dis = vec.length();
//                if(dis - minDis < 0.001){
//                    closeDiamondSet.insert(i);
//                    closeDiamondSet.insert(j);
//                    //printf("Insert\n");
//                }

//            }
//        }
//        vector<hbc::Point> tmpCenters = centers;
//        centers.clear();
//        for(i=0; i<tmpCenters.size(); i++){
//            if(closeDiamondSet.find(i)!=closeDiamondSet.end())
//                continue;
//            //printf("index:%d\n",i);
//            centers.push_back(tmpCenters[i]);
//        }
//        //printf("\n\n\n");

//    }

    setDtSearchRegionWidth();
    int minWidth = mDtSearchRegionWidth/2;
    vector<hbc::Point> tmpCenters;
    if(mPatternFgPixNum == 0){
        mPatternFgPixNum = calForegroundPix(templateMat1,mDtThreshold);
        mDtPixNumDiffer = mPatternFgPixNum*3/4;
        //printf("\nPixNum: %d,%d,%d\n",mPatternFgPixNum,mDtPixNumDiffer,mDtSearchRegionWidth);
    }


//    if(centers.size()>0 ){
//        for(unsigned int i=0; i<centers.size(); i++){
//            Point center1 = centers[i];
//            cv::Rect region1 = cv::Rect(center1.x()-minWidth,center1.y()-minWidth,2*minWidth,2*minWidth);
//            int lx=region1.tl().x;
//            int ly=region1.tl().y;
//            int rx=region1.br().x;
//            int ry=region1.br().y;
////            if(region1.tl().x<0 || region1.tl().y<0 || region1.br().x>src1.cols-2 || region1.br().y>src1.rows-2 )
////                continue;
//            if(lx<0)
//                lx = 0;
//            if(ly<0)
//                ly=0;
//            if(rx>src1.cols)
//                rx = src1.cols;
//            if(ry>src1.rows){
//                ry = src1.rows;
//            }
//            if(rx-lx>0 && ry-ly>0 && lx>=0 && ly>=0)
//                region1 = cv::Rect(lx,ly,rx-lx,ry-ly);

//            cv::Mat roi1 = cv::Mat(src1,region1);
//            //cv::imshow("ROI",roi1);
//            int num = calForegroundPix(roi1,mDtThreshold);
//            //std::cout<<"\nPix num: "<<num<<std::endl;
//            if(abs(num-mPatternFgPixNum)<mDtPixNumDiffer){
//                tmpCenters.push_back(centers[i]);
//            }
//        }
//        centers.clear();
//        centers = tmpCenters;
//        //std::cout<<"centers size: "<<centers.size()<<std::endl;
//    }

    cv::Rect rect(m_roi);

    for(unsigned int i=0; i<centers.size(); i++)
    {
        int x = centers[i].x()*2;
        int y = centers[i].y()*2;
        if(!rect.contains(cv::Point(x,y)))
            continue;
        if(x<rect.tl().x + templateMat.cols || y< rect.tl().y + templateMat.rows ||
                x>rect.br().x-templateMat.cols/2 || y>rect.br().y - templateMat.rows/2 )
            continue;
        m_centers.push_back(hbc::Point(x,y));
    }

    double tt =((double)cv::getTickCount()-t)/cv::getTickFrequency();
    //printf("Time: %.3f\n",tt);
    return m_centers;

}


const list<Point>& DiamondCircleDetecter::houghCircle(IplImage* image)
{

    //double t = cv::getTickCount();
    cv::Mat src(image);
    cv::Mat templateMat(m_pattern[0]);
    if(!src.data || !templateMat.data)
        return m_centers;

    vector<cv::Mat> srcDownVec = doPyrDown(src);
    vector<cv::Mat> templateVec = doPyrDown(templateMat);
    cv::Mat src1 = srcDownVec.back();
    cv::Mat templateMat1 = templateVec.back();

    int width = templateMat1.cols/2-5;

    int minR = width;
    int maxR = width+10;

    setMinRadius(minR);
    setMaxRadius(maxR);

    std::vector<cv::Vec3f> result = finder->findCircles(src1);


    std::vector<hbc::Point> centers;
    std::vector<cv::Vec3f>::const_iterator itc = result.begin();
    while(itc != result.end())
    {
        centers.push_back(hbc::Point((*itc)[0], (*itc)[1]));
        ++itc;
    }

    setDtSearchRegionWidth();
    int minWidth = mDtSearchRegionWidth/2;
    vector<hbc::Point> tmpCenters;
    if(mPatternFgPixNum == 0){
        mPatternFgPixNum = calForegroundPix(templateMat1,mDtThreshold);
        mDtPixNumDiffer = mPatternFgPixNum/2;
        //printf("\nPixNum: %d,%d,%d\n",mPatternFgPixNum,mDtPixNumDiffer,mDtSearchRegionWidth);
    }


    if(centers.size()>0 ){
        for(unsigned int i=0; i<centers.size(); i++){
            Point center1 = centers[i];
            cv::Rect region1 = cv::Rect(center1.x()-minWidth,center1.y()-minWidth,2*minWidth,2*minWidth);
            int lx=region1.tl().x;
            int ly=region1.tl().y;
            int rx=region1.br().x;
            int ry=region1.br().y;
//            if(region1.tl().x<0 || region1.tl().y<0 || region1.br().x>src1.cols-2 || region1.br().y>src1.rows-2 )
//                continue;
            if(lx<0)
                lx = 0;
            if(ly<0)
                ly=0;
            if(rx>src1.cols)
                rx = src1.cols;
            if(ry>src1.rows){
                ry = src1.rows;
            }
            if(rx-lx>0 && ry-ly>0 && lx>=0 && ly>=0)
                region1 = cv::Rect(lx,ly,rx-lx,ry-ly);

            cv::Mat roi1 = cv::Mat(src1,region1);
            //cv::imshow("ROI",roi1);
            int num = calForegroundPix(roi1,mDtThreshold);
            //std::cout<<"\nPix num: "<<num<<std::endl;
            if(num-mPatternFgPixNum<mDtPixNumDiffer){
                tmpCenters.push_back(centers[i]);
            }
        }
        centers.clear();
        centers = tmpCenters;
        //std::cout<<"centers size: "<<centers.size()<<std::endl;
    }

    cv::Rect rect(m_roi);

    for(unsigned int i=0; i<centers.size(); i++)
    {
        int x = centers[i].x()*2;
        int y = centers[i].y()*2;
        if(!rect.contains(cv::Point(x,y)))
            continue;
        if(x<rect.tl().x + templateMat.cols || y< rect.tl().y + templateMat.rows ||
                x>rect.br().x-templateMat.cols/2 || y>rect.br().y - templateMat.rows/2 )
            continue;
        m_centers.push_back(hbc::Point(x,y));
    }

    //double tt =((double)cv::getTickCount()-t)/cv::getTickFrequency();
    //printf("Time: %.3f\n",tt);
    return m_centers;

}


int DiamondCircleDetecter::calForegroundPix(const cv::Mat& img, int threshold){
    if(!img.data)
        return 0;
    int row = img.rows;
    int col = img.cols;
    int forgroundPixNum = 0;
    cv::Mat bw;
    cv::threshold(img,bw,threshold,255,cv::THRESH_BINARY);
    for(int j = 0; j < row; j++){
        uchar* data = bw.ptr<uchar>(j);
        for(int i=0; i<col; i++){
            if(data[i] == 255)
                forgroundPixNum++;
        }
    }
    //printf("num:%d\t",forgroundPixNum);
    return forgroundPixNum;
}

void DiamondCircleDetecter::setMinDist(int val)
{
    finder->setMinDist(val);
}

void DiamondCircleDetecter::setCannyHighThreshold(int val)
{
    finder->setCannyHighThreshold(val);
}

void DiamondCircleDetecter::setMinVotes(int val)
{
    finder->setMinVotes(val);
}

void DiamondCircleDetecter::setMaxRadius(int val)
{
    finder->setMaxRadius(val);
}

void DiamondCircleDetecter::setMinRadius(int val)
{
    finder->setMinRadius(val);
}
