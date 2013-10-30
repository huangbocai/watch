#ifndef WATCHCONFIG_H
#define WATCHCONFIG_H

#include "inifile.hh"
#include "iniwrite.hh"

#include <cv.h>
#include <sys/types.h>
#include <dirent.h>

class Mark2Param {
public:
    int load(const char* inifile);

    double pickup_offset_x(){return (pickRelx-pickupOffsetX);}
    double pickup_offset_y(){return (pickRely-pickupOffsetY);}
    double glue_offset_x(){return (glueRelx-glueOffsetX);}
    double glue_offset_y(){return (glueRely-glueOffsetY);}

    //common param
    double referenceX;
    double referenceY;
    double pickRelx;
    double pickRely;
    double camRelx;
    double camRely;
    double glueRelx;
    double glueRely;
    double pickupOffsetX;
    double pickupOffsetY;
    double glueOffsetX;
    double glueOffsetY;
    double kxx;
    double kxy;
    double kyy;
    double kyx;

    bool withCamera;
    int camGain;
    int camAutoBL;
    int camBL;
    int camADL;
    int camExposure;

    //velocity
    double fastVel;
    double slowVel;

    double degInc;

    char varFile[256];

    //param from differnt object
    char projectName[128];
};

class Project
{
public:
    Project();
    //ProjectManage(const char* projectName);
    void init(const char* projectName);
    int load(const char* projectName);
    void save_diamond_camera_param(int adl, int brightness, int contrast, int exposure);
    void save_watch_camera_param(int adl, int brightness, int contrast, int exposure);
    void save_diamond_search_area();
    void save_watch_search_area();
    void save_scan_param();
    void save_as(const char* projectName);
    void save_diamond_pattern(const IplImage* img);
    void save_watch_pattern(const IplImage* img);
    IplImage* get_diamond_pattern(){return patternD;}
    IplImage* get_watch_pattern(){return patternW;}
    const char* ini_file()const {return iniFile;}
    const char* project_dir()const {return projectDirectory;}
    const char* get_project_name()const {return mProjectName;}

#define ACCEPTER(PARAM,type) \
type get_##PARAM()           \
{return PARAM;}              \
void set_##PARAM(type val)   \
{PARAM=val;}

    ACCEPTER(brightnessD,int)
    ACCEPTER(contrastD,int)
    ACCEPTER(exposureD,int)
    ACCEPTER(adlD,int)

    ACCEPTER(brightnessW,int)
    ACCEPTER(contrastW,int)
    ACCEPTER(exposureW,int)
    ACCEPTER(adlW,int)

    ACCEPTER(scanRowNum,int)
    ACCEPTER(scanColNum,int)

    ACCEPTER(glueTime,double)
    ACCEPTER(afterGlueTime,double)
    ACCEPTER(setDiamondTime,double)
    ACCEPTER(getDiamondTime,double)

    ACCEPTER(pickupOffsetX,double)
    ACCEPTER(pickupOffsetY,double)
    ACCEPTER(glueOffsetX,double)
    ACCEPTER(glueOffsetY,double)

    ACCEPTER(scanRowDis,double)
    ACCEPTER(scanColDis,double)
    ACCEPTER(sendZD,double)
    ACCEPTER(pickupZD,double)

    ACCEPTER(glueZPos,double)
    ACCEPTER(setDiamondZPos,double)

    ACCEPTER(searcRectD,CvRect&)
    ACCEPTER(searcRectW,CvRect&)

    int brightnessD;
    int contrastD;
    int exposureD;
    int adlD;

    CvRect searcRectD;
    double scanStartPos[3];
    double scanEndPos[3];

    int scanRowNum;
    int scanColNum;

    double scanRowDis;
    double scanColDis;
    double sendZD;
    double pickupZD;

    int brightnessW;
    int contrastW;
    int exposureW;
    int adlW;

    CvRect searcRectW;
    double glueZPos;
    double setDiamondZPos; //镶钻高度

    double glueTime;
    double afterGlueTime;
    double setDiamondTime;
    double getDiamondTime;

    double pickupOffsetX;
    double pickupOffsetY;
    double glueOffsetX;
    double glueOffsetY;

private:
    IplImage* patternD;
    IplImage* patternW;
    char mProjectName[128];
    char projectDirectory[512];
    char patternFileD[512];
    char patternFileW[512];
    char iniFile[512];
};

class ProjectManage
{
public:
    typedef enum{SCAN_X0=3301, SCAN_Y0, SCAN_Z, SCAN_ROW_NUM, SCAN_COL_NUM, SCAN_ROW_DIS, SCAN_COL_DIS,
         PICKUP_Z,       //3308
         SETDIAMOND_Z,
         SET_GLUE_Z,
         GLUE_T,
         AFTER_GLUE_T,
         GET_DIAMOND_T,
         SET_DIAMOND_T
        } VAR_PARAM;
    ProjectManage();
    void load_project(const char* projectName);
    void save_as_project(const char* projectName);
    const char* get_current_project_name();

    void save_diamond_camera_param(int adl, int brightness, int contrast, int exposure);
    void save_watch_camera_param(int adl, int brightness, int contrast, int exposure);
    void save_diamond_search_area();
    void save_watch_search_area();
    void save_scan_param();
    void save_as(const char* projectName);
    void save_diamond_pattern(const IplImage* img);
    void save_watch_pattern(const IplImage* img);
    IplImage* get_diamond_pattern();
    IplImage* get_watch_pattern();
    const char* ini_file()const;
    const char* project_dir()const;
    void change_current_project(const char* projectName);

#define PARAM_ACCEPTER(AXIS,type) \
type get_##AXIS()                 \
{return currentPrj->get_##AXIS();}\
void set_##AXIS(type val)         \
{currentPrj->set_##AXIS(val);}

    PARAM_ACCEPTER(brightnessD,int)
    PARAM_ACCEPTER(contrastD,int)
    PARAM_ACCEPTER(exposureD,int)
    PARAM_ACCEPTER(adlD,int)

    PARAM_ACCEPTER(brightnessW,int)
    PARAM_ACCEPTER(contrastW,int)
    PARAM_ACCEPTER(exposureW,int)
    PARAM_ACCEPTER(adlW,int)

    PARAM_ACCEPTER(scanRowNum,int)
    PARAM_ACCEPTER(scanColNum,int)

    PARAM_ACCEPTER(glueTime,double)
    PARAM_ACCEPTER(afterGlueTime,double)
    PARAM_ACCEPTER(setDiamondTime,double)
    PARAM_ACCEPTER(getDiamondTime,double)

    PARAM_ACCEPTER(pickupOffsetX,double)
    PARAM_ACCEPTER(pickupOffsetY,double)
    PARAM_ACCEPTER(glueOffsetX,double)
    PARAM_ACCEPTER(glueOffsetY,double)

    PARAM_ACCEPTER(scanRowDis,double)
    PARAM_ACCEPTER(scanColDis,double)
    PARAM_ACCEPTER(sendZD,double)
    PARAM_ACCEPTER(pickupZD,double)

    PARAM_ACCEPTER(glueZPos,double)
    PARAM_ACCEPTER(setDiamondZPos,double)

    PARAM_ACCEPTER(searcRectD,CvRect&)
    PARAM_ACCEPTER(searcRectW,CvRect&)


    //double scanStartPos[3];
    void set_scanStartPos(int index, double val);
    double get_scanStartPos(int index);

    //double scanEndPos[3];
    void set_scanEndPos(int index, double val);
    double get_scanEndPos(int index);

private:
    Project* currentPrj;
};





#endif // WATCHCONFIG_H
