
#ifndef MARK_WIDGET_H
#define MARK_WIDGET_H

#define WITH_EMC

#include "ui_markWidget.h"
#include "photoCapture.hh"
#include "displayModel.h"
#include "markView.h"
#include "loadRecordDialog.h"
#include "newprojectdialog.h"
#include "markEMC.h"
#include "watchDetect.h"
#include "watchConfig.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <QtGui>

typedef struct {
    QPushButton* bt_adjustPos[4];
    QLabel* lb_adjPosX[4];
    QLabel* lb_adjPosY[4];
}MarkQtObjects;

class RecognizeResult
{
public:
    RecognizeResult():scanIndex(0),scanHoleIndex(0){pickupIterator=dimamondPos.begin();}
    list<Point> dimamondPos;
    list<Point>::const_iterator pickupIterator;
    int scanIndex;
    int scanHoleIndex;
};


class Mark2Adjust{
public:
    Mark2Adjust():posBitmap(0){}
    Point center[4];
    Point pos[4];
    ImageAdjust imageAdjust;
    char posBitmap;
};



class Position
{
public:
    Position()
    {
        for(int i=0; i<5; i++)
            val[i]=0;
    }
    Position(double xVal, double yVal, double zVal, double aVal, double bVal, RectangleFrame rect=RectangleFrame())
    {
        val[0] = xVal; val[1] = yVal; val[2] = zVal; val[3] = aVal; val[4] = bVal; searchArea = rect;
    }
    void set_value(double value, int valueIndex) {val[valueIndex] = value;}
    double get_value(int valueIndex)const {return val[valueIndex];}
    RectangleFrame get_search_area()const {return searchArea;}
    CvRect get_search_cv_area()const {return cvRect(searchArea.get_top_left().x(),searchArea.get_top_left().y(),
                                                 searchArea.get_width(),searchArea.get_height());}
    double x(){return val[0];}
    double y(){return val[1];}
    double z(){return val[2];}
    double a(){return val[3];}
    double b(){return val[4];}
    Position rotate(double deg){
        return Position(val[0],val[1],val[2],val[3],val[4]+deg,searchArea);
    }

    bool operator !=(const Position& pos) const
    {
        for(int i=0; i<5; i++){
            if(fabs(pos.get_value(i) - val[i])>0.00001)
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
    Recorder(string prjDir);
    void record_current_pos(double x, double y, double z, double a, double b, RectangleFrame rect);
    void record_current_pos(double pPos[5], RectangleFrame rect);
    void abandon_current_pos();
    void abandon_all_pos();
    void finish_record_watch_pos();
    void auto_record_watch_pos(int num, double deg);
    void finish_record_hole_pos();
    unsigned int get_pos_num(){return posVec.size();}
    void set_current_index(unsigned int index) {currentIndex = index;}
    void incr_current_index(unsigned int step){
        currentIndex += step;
    }
    unsigned int get_current_index(){return currentIndex;}
    //std::string get_file_name(){return fileName;}
    const Position* first_position();
    const Position* next_position();
    const Position* get_position(unsigned int index);
    //const Position& get_last_position(){return get_position(currentIndex-2);}
    void load();
    void clear_holes_pos() {holesPosVec.clear();}
    void set_mark_index(unsigned int index) {currentMarkIndex = index;}
    unsigned int get_mark_index() {return currentMarkIndex;}
    void incr_mark_index(unsigned int step){currentMarkIndex += step;}
    void set_hole_index(unsigned int index) {currentHoleIndex = index;}
    unsigned int get_hole_index() {return currentHoleIndex;}
    void incr_hole_index(unsigned int step){currentHoleIndex += step;}

    void set_glue_hole_index(unsigned int index) {currentGlueHoleIndex = index;}
    unsigned int get_glue_hole_index() {return currentGlueHoleIndex;}
    void incr_glue_hole_index(unsigned int step){currentGlueHoleIndex += step;}

    void set_glue_z_value(double zValue){glueZPos = zValue;}
    double get_glue_z_value() {return glueZPos;}

    void sort();

    void change_project(string prjDir);


private:
    bool is_file_open(ofstream& ofs, string fileName);

public:
    vector<Position> holesPosVec;
    vector<Position>::iterator holeIter;
private:
    unsigned int currentIndex;
    unsigned int currentMarkIndex;
    unsigned int currentHoleIndex;
    unsigned int currentGlueHoleIndex;
    string watchPosFileName;
    string holePosFileName;
    vector<Position> posVec;
    static const int lineLength = 128;
    Position firstPos;
    double glueZPos;
};

//for interface update
class Information
{
public:
    Information():watchPosIndex(0),gluePosIndex(0),holePosIndex(0),
        diamondNum(0),watchPosNum(0),holePosNum(0),endAutoRun(true),
        endSetGLue(true),endSetDiamond(true),endScanWatch(true),
        start(false),paused(false),stop(false),changeProject(false),runTime("00:00")
    {
        for(int i=0; i<4; i++)
            ioState[i] = false;
    }

    int watchPosIndex;
    int gluePosIndex;
    int holePosIndex;

    int diamondNum;
    int watchPosNum;
    int holePosNum;

    double setGlueZ;
    double setDiamondZ;
    double getDiamondZ;

    double glueT;
    double afterGlueT;
    double setDiamondT;
    double getDiamondT;

    double pickupOffsetX;
    double pickupOffsetY;
    double glueOffsetX;
    double glueOffsetY;

    double slowVel;
    double fastVel;

    bool ioState[5];
    bool endAutoRun;
    bool endSetGLue;
    bool endSetDiamond;
    bool endScanWatch;

    bool start;
    bool paused;
    bool stop;

    bool changeProject;

    QString runTime;

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

    //emc controler
    void machine_open(int cmd);
    void home();
    void zero();
    double get_fast_velocity(){return param.fastVel;}
    double get_slow_velocity(){return param.slowVel;}
    void set_fast_velocity(double vel);
    void set_slow_velocity(double vel);
    void jog(int axis, double velocity);
    void end_jog(int axis);
    void scan_diamon();
    void scan_watch();
    void set_glue();
    void set_diamond();
    void auto_run(bool type);
    void pause();
    void stop();
    void closeSystem();
    void set_var_param(int varNum, double value);
    void set_var_params(double* values);//update 7 var parameters's values
    void set_io(int index , bool on);
    void set_pickup_diamnod_z(double value);
    void set_glue_z_pos(double value);
    void set_setdiamond_z_pos(double value);
    void set_time(int index, int varNum, double value);
    void set_offset(int index, double value);

    //algorithm setting
    void set_diamond_detect_algorithm(int type);
    void set_dt_threshold(int value);
    int get_dt_threshold();
    void set_dt_pix_num_differ(int value);
    int get_dt_pix_num_differ();
    void set_dt_search_region_width(double value);
    double get_dt_search_region_width();
    void set_distance_between_diamonds(double value);

    //project
    void open_project();
    void new_project();

signals:
    void update_emc_status(const MarkEmcStatus& status);
    void update_infor(Information& infor);

private slots:
    //status bar
     void show_load_dialog();

    //diamod page
    void select_pattern_toggled(bool checked);
    void search_area_toggled(bool checked);
    void diamond_test_toggled(bool checked);
    void set_scan_beginning();
    void back_scan_beginning();
    void set_scan_end();
    void back_scan_end();
    void scan_test();
    void pickup_first();
    void pickup_next();
    void pickup_all();
    void choose_pattern_shap();
    //void record_diamond_pos();
    void on_bt_clearDiamond_clicked();

    //watch page
    void change_angle();
    void record_watch_pos();
    void abandon_current_pos();
    void abandon_all_pos();
    void finish_record_watch_pos();
    void auto_record_watch_pos();
    void get_first_pos();
    void get_next_pos();
    void cam_run();
    void set_first_hole();
    void set_next_hole();
    void set_all_holes();
    void set_diamond_in_all_holes();
    void auto_set_diamond();
    void half_auto_set_diamond();

    //image page
    void focus_point_select(int x, int y);
    void set_focus_times(bool checked);
    void load_image();
    void save_image();
    void set_camera_param(int val);
    void auto_brightness(int val);

    //adjust page
    void adjPos_pressed();
    void adjust_pressed();
    void adjust_clear_pressed();
    void detect_hole_presssed();
    void input_hole_pos_pressed();
    void input_glue_pos_pressed();
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
    void ready_for_watch_scan();
    void auto_detect_watch();
    void clear_hole_pos();
    void mark_view_update();
    void mark_adjust_param();

    void load_project(const char* projectName);
    void init_information();

    //拾起一颗石子
    void pickup_one_diamond(const Point& diamondPos, double pickupZ);
    //在一个槽里滴胶
    void set_one_glue(const Position& holePos, double setGlueZ);
    //在一个槽里放置石子
    void set_one_diamond(const Position& holePos, double setDiamondZ);
    //扫描表壳上的一个位置
    void scan_one_pos_of_watch(const Position* watchPos);

    MarkHal*halData;
    MarkEmcStatus emcStatus;
    RecognizeResult diamondPosition;

    Mark2Adjust adjust;
    ImageCapture* capture;
    Mark2Param param;
    ProjectManage prjManage;
    ImageActualTM* transfMatrix;
    IplImage* srcImage;

    Recorder* posRecorder;
    Information infor;

    DiamondCircleDetecter* diamondCirclesDetecter;
    WatchCircleDetecter* watchCircleDetecter;

    MarkQtObjects qtObjects;
    QWidget* adjTable;

    LoadRecordDialog* loadRecordDialog;
    NewProjectDialog* newProjectDialog;

    MarkView* markView;
    PatternView* patternView;
    PatternView* holePatternView;
    FocusAidView* focusAidView;
    QDoubleValidator* doubleValidator;
    QIntValidator* intValidator;
};
#endif
