
#include "watchConfig.h"
#include <highgui.h>

int Mark2Param::load(const char* iniFile)
{
    IniFile inifile;
    if (inifile.Open(iniFile) == false) {
		printf("can not open ini file %s\n",iniFile);
		return -1;
  	}
	//common ini param
	ini_get_double_param(&inifile, "MARK", "CAM_REL_SPINDLE_X", &camRelx, 0);
	ini_get_double_param(&inifile, "MARK", "CAM_REL_SPINDLE_Y", &camRely, 0);
	ini_get_double_param(&inifile, "MARK", "MM_PER_PIXEL_WW", &kxx, 0.004);
	ini_get_double_param(&inifile, "MARK", "MM_PER_PIXEL_WH", &kxy, 0);
	ini_get_double_param(&inifile, "MARK", "MM_PER_PIXEL_HH", &kyy, 0.004);
	ini_get_double_param(&inifile, "MARK", "MM_PER_PIXEL_HW", &kyx, 0);
    ini_get_double_param(&inifile, "MARK", "GLUE_REL_SPINDLE_X", &glueRelx, 0);
    ini_get_double_param(&inifile, "MARK", "GLUE_REL_SPINDLE_Y", &glueRely, 0);
    ini_get_int_param(&inifile, "MARK", "CAM_GAIN", &camGain, 0);
    ini_get_int_param(&inifile, "MARK", "CAM_ADC_LEVEL", &camADL, 0);
    ini_get_int_param(&inifile, "MARK", "CAM_BLACK_LEVEL", &camBL, 0);
    ini_get_int_param(&inifile, "MARK", "CAM_EXPOSURE", &camExposure, 0);
    ini_get_double_param(&inifile, "MARK", "DEGREE_INCREASE", &degInc, 0);
    ini_get_string_param(&inifile, "RS274NGC", "PARAMETER_FILE", varFile,"");
    ini_get_string_param(&inifile, "MARK", "CURRENT_PROJECT", projectName, "test1");
	inifile.Close();
	return 0;
}


ProjectManage::ProjectManage(){
    patternD=NULL;
    patternW=NULL;
}




int ProjectManage::load(const char* projectName){

    printf("projectName=%s\n", projectName);
    sprintf(projectDirectory, "/home/u/cnc/镶钻存档/%s", projectName);
    sprintf(iniFile, "%s/watch.ini", projectDirectory);
    sprintf(patternFileD, "%s/diamond.ppm", projectDirectory);
    sprintf(patternFileW, "%s/watch.ppm", projectDirectory);

    patternD=cvLoadImage(patternFileD, 0);
    patternW=cvLoadImage(patternFileW, 0);

    IniFile inifile;
    if (inifile.Open(iniFile) == false) {
        printf("can not open ini file %s\n",iniFile);
        return -1;
    }
    ini_get_int_param(&inifile, "DIAMOND", "BRIGHTNESS", &brightnessD, 0);
    ini_get_int_param(&inifile, "DIAMOND", "CONTRAST", &contrastD, 0);
    ini_get_int_param(&inifile, "DIAMOND", "EXPOSURE", &exposureD, 20);
    ini_get_int_param(&inifile, "DIAMOND", "ADL", &adlD, 0);
    int sx, sy, sw, sh;
    ini_get_int_param(&inifile, "DIAMOND", "SEARCH_LEFT", &sx, 20);
    ini_get_int_param(&inifile, "DIAMOND", "SEARCH_TOP", &sy, 20);
    ini_get_int_param(&inifile, "DIAMOND", "SEARCH_WIDTH", &sw, 200);
    ini_get_int_param(&inifile, "DIAMOND", "SEARCH_HEIGHT", &sh, 200);
    searcRectD= cvRect(sx, sy, sw, sh);


    ini_get_double_param(&inifile, "DIAMOND", "SCAN_X0", &scanStartPos[0], 0);
    ini_get_double_param(&inifile, "DIAMOND", "SCAN_Y0", &scanStartPos[1], 0);
    ini_get_double_param(&inifile, "DIAMOND", "SCAN_Z0", &scanStartPos[2], 0);
    ini_get_double_param(&inifile, "DIAMOND", "SCAN_ROW_DIS", &scanRowDis, 0);
    ini_get_double_param(&inifile, "DIAMOND", "SCAN_COL_DIS", &scanColDis, 0);
    ini_get_int_param(&inifile, "DIAMOND", "SCAN_ROW_NUM", &scanRowNum, 1);
    ini_get_int_param(&inifile, "DIAMOND", "SCAN_COL_NUM", &scanColNum, 1);

    ini_get_double_param(&inifile, "DIAMOND", "PICKUP_Z", &pickupZD, 0);
    ini_get_double_param(&inifile, "DIAMOND", "SEND_Z", &sendZD, 0);



    ini_get_int_param(&inifile, "WATCH", "BRIGHTNESS", &brightnessW, 0);
    ini_get_int_param(&inifile, "WATCH", "CONTRAST", &contrastW, 0);
    ini_get_int_param(&inifile, "WATCH", "EXPOSURE", &exposureW, 20);
    ini_get_int_param(&inifile, "WATCH", "ADL", &adlW, 0);

    ini_get_int_param(&inifile, "WATCH", "SEARCH_LEFT", &sx, 20);
    ini_get_int_param(&inifile, "WATCH", "SEARCH_TOP", &sy, 20);
    ini_get_int_param(&inifile, "WATCH", "SEARCH_WIDTH", &sw, 200);
    ini_get_int_param(&inifile, "WATCH", "SEARCH_HEIGHT", &sh, 200);
    searcRectW= cvRect(sx, sy, sw, sh);

    ini_get_double_param(&inifile,"WATCH", "GLUE_Z_POS",&glueZPos,0);

    return 0;
}

void ProjectManage::save_diamond_pattern(const IplImage *img){
    cvSaveImage(patternFileD, img);
}

void ProjectManage::save_watch_pattern(const IplImage *img){
    cvSaveImage(patternFileW, img);
}

void ProjectManage::save_diamond_camera_param(int adl, int brightness, int contrast, int exposure){
    adlD=adl;
    brightnessD=brightness;
    contrastD=contrast;
    exposureD=exposure;
    write_profile_int("DIAMOND", "BRIGHTNESS",brightnessD, iniFile);
    write_profile_int("DIAMOND", "CONTRAST",contrastD, iniFile);
    write_profile_int("DIAMOND", "EXPOSURE",exposureD, iniFile);
    write_profile_int("DIAMOND", "ADL",adlD, iniFile);
}

void ProjectManage::save_watch_camera_param(int adl, int brightness, int contrast, int exposure)
{
    adlW=adl;
    brightnessW=brightness;
    contrastW=contrast;
    exposureW=exposure;
    write_profile_int("WATCH", "BRIGHTNESS",brightnessW, iniFile);
    write_profile_int("WATCH", "CONTRAST",contrastW, iniFile);
    write_profile_int("WATCH", "EXPOSURE",exposureW, iniFile);
    write_profile_int("WATCH", "ADL",adlW, iniFile);
}

void ProjectManage::save_diamond_search_area(){
    write_profile_int("DIAMOND", "SEARCH_LEFT", searcRectD.x, iniFile);
    write_profile_int("DIAMOND", "SEARCH_TOP", searcRectD.y, iniFile);
    write_profile_int("DIAMOND", "SEARCH_WIDTH", searcRectD.width, iniFile);
    write_profile_int("DIAMOND", "SEARCH_HEIGHT", searcRectD.height, iniFile);
}

void ProjectManage::save_watch_search_area(){
    write_profile_int("WATCH", "SEARCH_LEFT", searcRectW.x, iniFile);
    write_profile_int("WATCH", "SEARCH_TOP", searcRectW.y, iniFile);
    write_profile_int("WATCH", "SEARCH_WIDTH", searcRectW.width, iniFile);
    write_profile_int("WATCH", "SEARCH_HEIGHT", searcRectW.height, iniFile);
}

void ProjectManage::save_scan_param(){
    write_profile_double("DIAMOND", "SCAN_X0", scanStartPos[0], iniFile);
    write_profile_double("DIAMOND", "SCAN_Y0", scanStartPos[1], iniFile);
    write_profile_double("DIAMOND", "SCAN_Z0", scanStartPos[2], iniFile);
    write_profile_double("DIAMOND", "SCAN_ROW_DIS", scanRowDis, iniFile);
    write_profile_double("DIAMOND", "SCAN_COL_DIS", scanColDis, iniFile);
    write_profile_int("DIAMOND", "SCAN_ROW_NUM", scanRowNum, iniFile);
    write_profile_int("DIAMOND", "SCAN_COL_NUM", scanColNum, iniFile);
}


