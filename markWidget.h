
#ifndef MARK_WIDGET_H
#define MARK_WIDGET_H

#define WITH_EMC

#include "ui_markWidget.h"
#include "photoCapture.hh"
#include "displayModel.h"
#include "markView.h"
#include "loadRecordDialog.h"
#include "markEMC.h"
#include "watchDetect.h"
#include "watchConfig.h"
#include <iostream>
#include <fstream>
#include <vector>

typedef struct {
    QPushButton* bt_adjustPos[4];
    QLabel* lb_adjPosX[4];
    QLabel* lb_adjPosY[4];
}MarkQtObjects;

class WatchResult
{
public:
    WatchResult():scanIndex(0){pickupIterator=dimamondPos.begin();}
    list<Point> dimamondPos;
    list<Point>::const_iterator pickupIterator;
    int scanIndex;
};


class Mark2Adjust{
public:
    Mark2Adjust():posBitmap(0){}
    Point center[4];
    Point pos[4];
    ImageAdjust imageAdjust;
    char posBitmap;
};

class OffsetSetting
{
public:
    OffsetSetting(double ccdRelX, double ccdRelY, double glueRelX, double glueRely):
        holePosOk(false), holeDetectOk(false){
        ccdOffset[0]=ccdRelX; ccdOffset[1]=ccdRelY; glueOffset[0]=glueRelX; glueOffset[1]=glueRely;}
    double holePos[2];
    bool holePosOk;
    double holeDetePos[2];
    bool holeDetectOk;
    double ccdOffset[2];
    double gluePos[2];
    double glueOffset[2];
};

class Position
{
public:
    Position()
    {
        val[0] = 0; val[1] = 0; val[2] = 0; val[3] = 0; val[4] = 0;
    }
    Position(double xVal, double yVal, double zVal, double aVal, double bVal, RectangleFrame rect)
    {
        val[0] = xVal; val[1] = yVal; val[2] = zVal; val[3] = aVal; val[4] = bVal; searchArea = rect;
    }
    void set_value(double value, int valueIndex) {val[valueIndex] = value;}
    double get_value(int valueIndex)const {return val[valueIndex];}
    RectangleFrame get_search_area()const {return searchArea;}
    double x(){return val[0];}
    double y(){return val[1];}
    double z(){return val[2];}
    double a(){return val[3];}
    double b(){return val[4];}
    bool operator !=(const Position& pos) const
    {
        for(int i=0; i<5; i++){
            if(pos.get_value(i) != val[i])
                return true;
        }
        return false;
    }
private:
    double val[5];
    RectangleFrame searchArea;
};

class Recorder
{
public:
    Recorder(std::string prjDir);
    void record_current_pos(double x, double y, double z, double a, double b, RectangleFrame rect);
    void record_current_pos(double pPos[5], RectangleFrame rect);
    void abandon_current_pos();
    void abandon_all_pos();
    void finish_record_cam_pos();
    unsigned int get_pos_num(){return posVec.size();}
    void set_current_index(unsigned int index) {currentIndex = index;}
    void incr_current_index(unsigned int step){
        currentIndex += step;
    }
    unsigned int get_current_index(){return currentIndex;}
    std::string get_file_name(){return fileName;}    
    const Position* first_position();
    const Position* next_position();
    void load();
    bool is_the_last_pos(){return currentIndex-2>=posVec.size()?true:false;}
    const Position* get_position(unsigned int index);
    const Position* get_last_position(){return get_position(currentIndex-2);}
private:
    bool is_file_open(ofstream& ofs);

private:
    unsigned int currentIndex;
    std::string fileName;
    std::vector<Position*> posVec;
    static const int lineLength = 64;
    const Position* lastPos;
};


class MarkWidget: public QWidget, public Ui::MarkWidget
{
    Q_OBJECT
public:
    MarkWidget(int argc,  char **argv, QWidget* parent=0);
//    const QImage& get_view_image(int width, int height);
    const char* get_version();
    const char* get_current_project()const;
   ~MarkWidget();

private slots:  

    //status bar
     void show_load_dialog();

    //diamod page
    void select_pattern_toggled(bool checked);
    void search_area_toggled(bool checked);
    void diamond_test_toggled(bool checked);
    void set_scan_beginning();
    void back_scan_beginning();
    void scan_test();
    void set_pickup_diamnod_z();
    void set_send_diamnod_z();
    void pickup_first();
    void pickup_next();
    void pickup_all();

    //watch page
    void change_angle();
    void record_cam_pos();
    void abandon_current_pos();
    void abandon_all_pos();
    void finish_record_cam_pos();
    void get_first_pos();
    void get_next_pos();
    void cam_run();

    //image page
    void focus_point_select(int x, int y);
    void set_focus_times(bool checked);
    void load_image();
    void save_image();
    void set_camera_param(int val);

    //adjust page
    void adjPos_pressed();
    void adjust_pressed();
    void adjust_clear_pressed();
    void detect_hole_presssed();
    void input_hole_pos_pressed();
    void input_glue_pos_pressed();
    void set_ccd_offset_pressed();
    void exit_adjust_intf();

    //timer update
    void view_update();
    void slow_cycle();
    void fast_react_cycle();
    void cv_cmd_cycle();

private:
    void get_qt_objects();
    void start_Capture(int contrastVal, int brightnessVal, int exposureVal, int ADLevel, int bufferNum=2);
    void detecter_model_init();

    void status_bar_init();
    void diamond_page_init();
    void watch_page_init();
    void image_page_init();
    void adjust_page_init();

    void ready_for_diamond_scan();
    void auto_detect_diamond();
    void clear_diamond_pos();
    void auto_detect_watch();

    void mark_view_update();


    void mark_adjust_param();
    int detectHole_pressed(int type, double& cx, double& cy);

    MarkHal*halData;
    MarkEmcStatus emcStatus;
    WatchResult watchResult;

    Mark2Adjust adjust;
    OffsetSetting* offsetSetting;
    ImageCapture* capture;
    Mark2Param param;
    ProjectManage prjManage;
    ImageActualTM* transfMatrix;
    IplImage* srcImage;

    Recorder* posRecorder;

    CirclesDetecter* circlesDetecter;

    MarkQtObjects qtObjects;
    QWidget* adjTable;

    LoadRecordDialog* loadRecordDialog;

    MarkView* markView;
    PatternView* patternView;
    FocusAidView* focusAidView;
    QDoubleValidator* doubleValidator;
    QIntValidator* intValidator;

};

#endif
