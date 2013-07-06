
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