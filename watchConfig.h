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
//    int load_project(const char* directory);
//    void save_sim();
//    int save_project(const char* directory);



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

class ProjectManage
{
public:
    typedef enum{SCAN_X0=3301, SCAN_Y0, SCAN_Z, SCAN_ROW_NUM, SCAN_COL_NUM, SCAN_ROW_DIS, SCAN_COL_DIS,
         PICKUP_Z,
         SETDIAMOND_Z,
         SET_GLUE_Z,
         GLUE_T,
         AFTER_GLUE_T,
         GET_DIAMOND_T,
         SET_DIAMOND_T
        } VAR_PARAM;
    ProjectManage();
    //ProjectManage(const char* projectName);
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

    int brightnessD;
    int contrastD;
    int exposureD;
    int adlD;
    CvRect searcRectD;
    double scanStartPos[3];
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

private:
    IplImage* patternD;
    IplImage* patternW;
    char projectDirectory[512];
    char patternFileD[512];
    char patternFileW[512];
    char iniFile[512];
};




#endif // WATCHCONFIG_H
