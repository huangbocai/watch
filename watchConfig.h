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

    //velocity
    double fastVel;
    double slowVel;
    void save_fast_velocity(const char* iniFile);
    void save_slow_velocity(const char* iniFile);

    //common param
    double referenceX;
    double referenceY;
    double pickRelx;
    double pickRely;
    double glueRelx;
    double glueRely;
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

    double degInc;

    char varFile[256];

    //param from differnt object
    char projectName[128];


};

class ProjectManage
{
public:
    enum{SCAN_X0=3301, SCAN_Y0, SCAN_Z, SCAN_ROW_NUM, SCAN_COL_NUM, SCAN_ROW_DIS, SCAN_COL_DIS, PICKUP_Z, SEND_ZD};
    ProjectManage();
    //ProjectManage(const char* projectName);
    int load(const char* projectName);
    void save_diamond_camera_param(int adl, int brightness, int contrast, int exposure);
    void save_diamond_search_area();
    void save_scan_param();
    void save_as(const char* projectName);
    void save_diamond_pattern(const IplImage* img);
    void save_watch_camera_param(int adl, int brightness, int contrast, int exposure);
    void save_watch_pattern(const IplImage* img);
    void save_watch_search_area();
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

private:
    IplImage* patternD;
    IplImage* patternW;
    char projectDirectory[512];
    char patternFileD[512];
    char patternFileW[512];
    char iniFile[512];
};




#endif // WATCHCONFIG_H
