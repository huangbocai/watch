
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
