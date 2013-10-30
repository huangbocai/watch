#include <QDebug>
#include <QtGui>
#include "markWidget.h"
#include "emcsec.hh"
#include "iniwrite.hh"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <algorithm>
//#include <vector>
//#include <iostream>

#define MARK2_VERSION "WATCH 1.0.1"

#define OFFSET_WHITE_CIRCLE 0
#define OFFSET_BLACK_CIRCLE 1
#define OFFSET_CENTER	2
#define CCDS_RELATE 3

Recorder::Recorder(string prjDir)
{
    watchPosFileName = prjDir + string("/Watch.pos");
    holePosFileName = prjDir + string("/holes.pos");
    currentIndex = 0;
    currentMarkIndex = 0;
    currentHoleIndex = 0;
    currentGlueHoleIndex = 0;
    posVec.clear();
}

void Recorder::change_project(string prjDir)
{
    watchPosFileName = prjDir + string("/Watch.pos");
    holePosFileName = prjDir + string("/holes.pos");
    currentIndex = 0;
    currentMarkIndex = 0;
    currentHoleIndex = 0;
    currentGlueHoleIndex = 0;
    posVec.clear();
    load();
}

void Recorder::record_current_pos(double x, double y, double z, double a, double b ,RectangleFrame rect)
{
    Position pos (x,y,z,a,b,rect);
    if(posVec.size()>0){
        Position currentPos = posVec.back();
        if(currentPos != pos){
            posVec.push_back(pos);
        }
        else{
            cout<<"This position had aready recorded!"<<endl;
            //delete pos;
        }
    }
    else
        posVec.push_back(pos);
}

void Recorder::record_current_pos(double pPos[5], RectangleFrame rect)
{
    record_current_pos(pPos[0],pPos[1],pPos[2],pPos[3],pPos[4],rect);
}

void Recorder::abandon_current_pos()
{
    posVec.pop_back();
}

void Recorder::abandon_all_pos()
{
    string tmpCmdStr = string("rm -r ")+watchPosFileName;
    int result = system(tmpCmdStr.c_str());
    if (result != 0) {
        printf("ERROR: delete %s fail\n", tmpCmdStr.c_str());
        //exit(-1);
    }
    tmpCmdStr = string("rm -r ")+holePosFileName;
    result = system(tmpCmdStr.c_str());
    if(result != 0){
        printf("ERROR: delete %s fail\n", tmpCmdStr.c_str());
    }

    posVec.clear();
    currentIndex = 0;
    holesPosVec.clear();
}

void Recorder::finish_record_watch_pos()
{
    ofstream ofs;
    vector<Position>::iterator iter;
    bool isFileOpen = is_file_open(ofs,watchPosFileName);

    if(isFileOpen){
        for(iter = posVec.begin(); iter != posVec.end();iter++){
            RectangleFrame rect = iter->get_search_area();
            ofs<<iter->x()<<" "<<iter->y()<<" "<<iter->z()<<" "
              <<iter->a()<<" "<<iter->b()<<" "
             <<rect.get_top_left().x()<<" "<<rect.get_top_left().y()<<" "
            <<rect.get_width()<<" "<<rect.get_height()<<endl;
        }
        ofs.close();
    }
}

void Recorder::auto_record_watch_pos(int num, double deg)
{
    Position firstPos = posVec[0];
    for(int i=1; i<num; i++){
        Position pos = firstPos.rotate(deg*i);
        posVec.push_back(pos);
    }

    finish_record_watch_pos();
}

void Recorder::finish_record_hole_pos()
{
    ofstream ofs;
    vector<Position>::iterator iter;
    bool isFileOpen = is_file_open(ofs,holePosFileName);
    sort();    
    if(isFileOpen){
        for(iter = holesPosVec.begin(); iter != holesPosVec.end(); iter++){
            ofs<<iter->x()<<" "<<iter->y()<<" "<<iter->z()<<" "
              <<iter->a()<<" "<<iter->b()<<endl;
        }
    }
    ofs.close();
}

bool Recorder::is_file_open(ofstream &ofs, string fileName)
{
    ofs.open(fileName.c_str());
    if(!ofs.is_open()){
        cout<<"open file "<<fileName<<" fail!"<<endl;
        return false;
    }
    ofs.precision(6);
    return true;
}

const Position* Recorder::first_position()
{
    currentIndex = 0;
    return get_position(currentIndex++);
}

const Position* Recorder::next_position()
{
    if(currentIndex>=posVec.size())
        currentIndex = 0;
    return get_position(currentIndex++);
}

const Position* Recorder::get_position(unsigned int index)
{
    if(index<posVec.size())
        return &posVec[index];
    else
        return NULL;
}


void Recorder::load()
{
    ifstream ifs;
    char buf[lineLength];
    QString lineStr;
    QStringList ld;

    ifs.open(watchPosFileName.c_str());
    if(ifs.fail()){
        cout<<"open file "<<watchPosFileName<<" fail!"<<endl;
        return ;
    }

    ifs.getline(buf,lineLength);

    while(!ifs.eof())
    {
        lineStr = QString(buf);
        ld = lineStr.split(" ",QString::SkipEmptyParts);
        double x,y,z,a,b,tpx,tpy,w,h;
        x = ld[0].toDouble(); y = ld[1].toDouble(); z = ld[2].toDouble();
        a = ld[3].toDouble(); b = ld[4].toDouble(); tpx = ld[5].toDouble();
        tpy = ld[6].toDouble(); w = ld[7].toDouble(); h = ld[8].toDouble();
        Position pos(x,y,z,a,b,RectangleFrame(Point(tpx,tpy),w,h));
        posVec.push_back(pos);        
        ifs.getline(buf,lineLength);
    }
    ifs.close();

    ifs.open(holePosFileName.c_str());
    if(ifs.fail()){
        cout<<"open file "<<holePosFileName<<" fail!"<<endl;
        return ;
    }

    ifs.getline(buf,lineLength,'\n');

    while(!ifs.eof())
    {
        lineStr = QString(buf);
        ld = lineStr.split(" ",QString::SkipEmptyParts);
        double x,y,z,a,b;
        x = ld[0].toDouble(); y = ld[1].toDouble(); z = ld[2].toDouble();
        a = ld[3].toDouble(); b = ld[4].toDouble();
        Position pos(x,y,z,a,b);
        holesPosVec.push_back(pos);
        ifs.getline(buf,lineLength,'\n');
    }
    ifs.close();
}

void Recorder::sort()
{
    unsigned int i,j, g=0;
    Position pos;
    Position pos1;
    Position tmpPos;
    double tmpB,posB,tmpL;

    //printf("\nsize: %d\n\n",holesPosVec.size());
    if(holesPosVec.size()<=1)
        return;

    for(i=0; i<holesPosVec.size()-1; i++){
        pos = holesPosVec[i];
        Vector2 v1(Point(0,0),Point(pos.x(),pos.y()));
        tmpB = fabs(pos.b());
        tmpL = v1.length();
        g = i;
        for(j = i+1; j<holesPosVec.size();j++){
            pos1 = holesPosVec[j];
            posB = fabs(pos1.b());
            Vector2 v2(Point(0,0),Point(pos1.x(),pos1.y()));
            if(tmpB-posB>0.0001){
                g = j;
                tmpB = posB;
                tmpL = v2.length();
            }
            else if(fabs(tmpB - posB)<0.0001){
                if(tmpL>v2.length()){
                    g = j;
                    tmpB = posB;
                    tmpL = v2.length();
                }
            }

        }
        tmpPos = holesPosVec[i];
        holesPosVec[i] = holesPosVec[g];
        holesPosVec[g] = tmpPos;
    }
}



MarkWidget::MarkWidget(int argc,  char **argv, QWidget* parent)
    :QWidget(parent)
{
    int res;
    printf("start mark %s\n", MARK2_VERSION);
#ifdef WITH_EMC
    halData = new MarkHal;
    res=emc_init(argc, argv);
    if(res){
        printf("emc initial fail\n");
        exit(-1);
     };
     param.load(EMC_INIFILE);
     //printf("inifile : %s\ntest!!!!!!!\n",EMC_INIFILE);
#else
    param.load("/home/u/cnc/configs/ppmc/ppmc.ini");
#endif
    prjManage.load_project(param.projectName);

    setupUi(this);
    get_qt_objects();
    start_Capture(param.camGain, param.camBL,param.camExposure,param.camADL, 2);
    detecter_model_init();

    markView= new MarkView(widgetMarkView);
    loadRecordDialog = new LoadRecordDialog(this);
    newProjectDialog = new NewProjectDialog(this);

    posRecorder = new Recorder(string(prjManage.project_dir()));
    posRecorder->load();
    init_information();

    status_bar_init();
    diamond_page_init();
    watch_page_init();
    image_page_init();
    adjust_page_init();    


    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(view_update()));
    timer->start(100);

    QTimer * slowTimer = new QTimer(this);
    connect(slowTimer, SIGNAL(timeout()), this, SLOT(slow_cycle()));
    slowTimer->start(120);


#ifdef WITH_EMC
    QTimer * halTimer = new QTimer(this);
    connect(halTimer, SIGNAL(timeout()), this, SLOT(fast_react_cycle()));
    halTimer->start(10);
#endif
}

MarkWidget::~MarkWidget(){
    emc_quit();
    hal_exit(halData->comp_id);
    printf("mark exit\n");
}

void MarkWidget::init_information()
{
    infor.setGlueZ = prjManage.get_glueZPos();
    infor.setDiamondZ = prjManage.get_setDiamondZPos();
    infor.getDiamondZ = prjManage.get_pickupZD();
    infor.glueT = prjManage.get_glueTime();
    infor.afterGlueT = prjManage.get_afterGlueTime();
    infor.getDiamondT = prjManage.get_getDiamondTime();
    infor.setDiamondT = prjManage.get_setDiamondTime();
    infor.pickupOffsetX = prjManage.get_pickupOffsetX();
    infor.pickupOffsetY = prjManage.get_pickupOffsetY();
    infor.glueOffsetX = prjManage.get_glueOffsetX();
    infor.glueOffsetY = prjManage.get_glueOffsetY();
    infor.slowVel = param.slowVel;
    infor.fastVel = param.fastVel;

}


//const QImage& MarkWidget::get_view_image(int width, int height){
//    return markView->get_view_image(width, height);
//}

const char* MarkWidget::get_version(){
    static const char* version = MARK2_VERSION;
    return version;
}

const char* MarkWidget::get_current_project()const{
    return param.projectName;
}




void MarkWidget::get_qt_objects(){
    qtObjects.bt_adjustPos[0]=bt_adjustPos0;
    qtObjects.bt_adjustPos[1]=bt_adjustPos1;
    qtObjects.bt_adjustPos[2]=bt_adjustPos2;
    qtObjects.bt_adjustPos[3]=bt_adjustPos3;
    qtObjects.lb_adjPosX[0]=lb_adjPosX0;
    qtObjects.lb_adjPosX[1]=lb_adjPosX1;
    qtObjects.lb_adjPosX[2]=lb_adjPosX2;
    qtObjects.lb_adjPosX[3]=lb_adjPosX3;
    qtObjects.lb_adjPosY[0]=lb_adjPosY0;
    qtObjects.lb_adjPosY[1]=lb_adjPosY1;
    qtObjects.lb_adjPosY[2]=lb_adjPosY2;
    qtObjects.lb_adjPosY[3]=lb_adjPosY3;
}

void MarkWidget::start_Capture(int contrastVal, int brightnessVal, int exposureVal, int ADLevel, int bufferNum)
{
    int res=0, width, height;
    capture= new ImageCapture(contrastVal, brightnessVal, exposureVal, ADLevel, bufferNum);
    assert(capture);
    res=capture->start();
    if(!res)
    {
        //camera ok, create src image buf
        printf("open camera ok\n");
        capture->get_image_size(width, height);
        srcImage=cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
        //param.withCamera=true;
    }
    else{
        //camera fail, load image from file for debug
        //   param.withCamera=false;
        delete capture;
        capture=NULL;
        //printf("Loading image /home/u/watch.ppm...\n\n");
        srcImage=cvLoadImage("/home/u/watch.ppm",0);
        if(srcImage==NULL){
            printf("can  not open /home/u/watch.ppm\n");
            width=1280; height=1024;
            srcImage=cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
            memset(srcImage->imageData, 0, width*height*sizeof(char));
        }
    }
}

void MarkWidget::detecter_model_init(){
    transfMatrix=new ImageActualTM( TransformMatrix(param.kxx, param.kxy, param.kyx, param.kyy),
        srcImage->width, srcImage->height, 0, 0);
    diamondCirclesDetecter = new DiamondCircleDetecter;
    diamondCirclesDetecter->set_level(1);
    diamondCirclesDetecter->set_similar(0.6);
    diamondCirclesDetecter->set_pattern(prjManage.get_diamond_pattern());
//    diamondCirclesDetecter->set_area(&prjManage.searcRectD);
    watchCircleDetecter = new WatchCircleDetecter;
    watchCircleDetecter->set_pattern(prjManage.get_watch_pattern());
}

void MarkWidget::status_bar_init(){
    lb_project->setText(QString::fromUtf8(param.projectName));
    lb_version->setText(MARK2_VERSION); 
    connect(bt_load, SIGNAL(clicked()), this, SLOT(show_load_dialog()));
}

void MarkWidget::diamond_page_init(){

    int w=patternViewFrame0->width();
    int h=patternViewFrame0->height();
    patternView = new PatternView(w, h, patternViewFrame0);
    rb_circle->setChecked(true);
    //rb_rectangle->setChecked(true);
    char buf[16];
    sprintf(buf, "%8.3f", prjManage.get_scanStartPos(0));
    lb_scanX0->setText(QString::fromUtf8(buf));
    sprintf(buf, "%8.3f", prjManage.get_scanStartPos(1));
    lb_scanY0->setText(QString::fromUtf8(buf));
    sprintf(buf, "%8.3f", prjManage.get_scanStartPos(2));
    lb_scanZ0->setText(QString::fromUtf8(buf));
    sprintf(buf, "%8.3f", prjManage.get_scanEndPos(0));
    lb_scanX1->setText(QString::fromUtf8(buf));
    sprintf(buf, "%8.3f", prjManage.get_scanEndPos(1));
    lb_scanY1->setText(QString::fromUtf8(buf));
    sprintf(buf, "%8.3f", prjManage.get_scanEndPos(2));
    lb_scanZ1->setText(QString::fromUtf8(buf));

    doubleValidator = new QDoubleValidator(this);
    intValidator = new QIntValidator(this);
    intValidator->setBottom(1);

    connect(tb_selectPattern0, SIGNAL(toggled(bool)), this, SLOT(select_pattern_toggled(bool)));
    connect(tb_searchArea0, SIGNAL(toggled(bool)), this, SLOT(search_area_toggled(bool)));
    connect(bt_detect0,SIGNAL(toggled(bool)), this, SLOT(diamond_test_toggled(bool)));
    connect(bt_setScanStart, SIGNAL(clicked()), this, SLOT(set_scan_beginning()));
    connect(bt_gotoScanStart, SIGNAL(clicked()), this, SLOT(back_scan_beginning()));
    connect(bt_setScanEnd, SIGNAL(clicked()), this, SLOT(set_scan_end()));
    connect(bt_gotoScanEnd, SIGNAL(clicked()), this, SLOT(back_scan_end()));
    connect(bt_scanTest,SIGNAL(clicked()), this, SLOT(scan_test()));
    connect(bt_pickupFirst, SIGNAL(clicked()), this, SLOT(pickup_first()));
    connect(bt_pickupNext, SIGNAL(clicked()), this, SLOT(pickup_next()));
    connect(bt_pickupAll, SIGNAL(clicked()), this, SLOT(pickup_all()));
    connect(rb_circle,SIGNAL(clicked()),this,SLOT(choose_pattern_shap()));
    connect(rb_rectangle,SIGNAL(clicked()),this,SLOT(choose_pattern_shap()));
    //connect(bt_recordD,SIGNAL(clicked()),this,SLOT(record_diamond_pos()));
}

void MarkWidget::watch_page_init()
{
    char buf[16];
    int posNum = 0;
    le_rotateDeg->setValidator(doubleValidator);
    sprintf(buf, "%.3f",param.degInc);
    le_rotateDeg->setText(QString::fromUtf8(buf));

    int w=patternViewFrame1->width();
    int h=patternViewFrame1->height();
    holePatternView = new PatternView(w, h, patternViewFrame1);

    posNum = posRecorder->get_pos_num();
    if(posNum<=0){
        bt_recordCamPos->setEnabled(true);
        bt_abandonCurrentPos->setEnabled(false);
        bt_abandonAllPos->setEnabled(false);
        bt_finishRecord->setEnabled(false);
        bt_autoRecord->setEnabled(false);
        bt_firstPos->setEnabled(false);
        bt_nextPos->setEnabled(false);
        bt_camRun->setEnabled(false);        
    }
    else{
        bt_recordCamPos->setEnabled(false);
        bt_abandonCurrentPos->setEnabled(false);
        bt_abandonAllPos->setEnabled(true);
        bt_finishRecord->setEnabled(false);
        bt_autoRecord->setEnabled(false);
        bt_firstPos->setEnabled(true);
        bt_nextPos->setEnabled(true);
        bt_camRun->setEnabled(true);
    }


    connect(tb_selectHolePattern,SIGNAL(toggled(bool)),this,SLOT(select_pattern_toggled(bool)));
    connect(tb_searchHoleArea,SIGNAL(toggled(bool)),this,SLOT(search_area_toggled(bool)));
    connect(bt_detectHoles,SIGNAL(toggled(bool)),this,SLOT(diamond_test_toggled(bool)));
    connect(bt_rotateForward, SIGNAL(clicked()), this, SLOT(change_angle()));
    connect(bt_rotateBackward, SIGNAL(clicked()), this, SLOT(change_angle()));
    connect(bt_recordCamPos,SIGNAL(clicked()),this,SLOT(record_watch_pos()));
    connect(bt_abandonCurrentPos,SIGNAL(clicked()),this,SLOT(abandon_current_pos()));
    connect(bt_abandonAllPos,SIGNAL(clicked()),this,SLOT(abandon_all_pos()));
    connect(bt_finishRecord,SIGNAL(clicked()),this,SLOT(finish_record_watch_pos()));
    connect(bt_autoRecord,SIGNAL(clicked()),this,SLOT(auto_record_watch_pos()));
    connect(bt_firstPos,SIGNAL(clicked()),this,SLOT(get_first_pos()));
    connect(bt_nextPos,SIGNAL(clicked()),this,SLOT(get_next_pos()));
    connect(bt_camRun,SIGNAL(clicked()),this,SLOT(cam_run()));
    connect(bt_setFirstHole,SIGNAL(clicked()),this,SLOT(set_first_hole()));
    connect(bt_setNextHole,SIGNAL(clicked()),this,SLOT(set_next_hole()));
    connect(bt_setAllHoles,SIGNAL(clicked()),this,SLOT(set_all_holes()));

}


void MarkWidget::image_page_init(){

    focusAidView= new FocusAidView(focusFrame->width(), focusFrame->height(), 4, focusFrame);
    bt_4x->setChecked(true);
    lb_contrast->setText(QString::number(param.camGain));
    lb_brightness->setText(QString::number(param.camBL));
    lb_exposure->setText(QString::number(param.camExposure));
    sp_ADLevle->setValue(param.camADL);
    sl_contrast->setValue(param.camGain);
    sl_brightness->setValue(param.camBL);
    sl_exposure->setValue(param.camExposure);
    if(capture)
        bt_loadImage->hide();


    connect(bt_4x, SIGNAL(toggled(bool)), this, SLOT(set_focus_times(bool)));
    connect(bt_8x, SIGNAL(toggled(bool)), this, SLOT(set_focus_times(bool)));
    connect(sp_ADLevle, SIGNAL(valueChanged(int)), this, SLOT(set_camera_param(int)));
    connect(sl_contrast, SIGNAL(valueChanged(int)), this, SLOT(set_camera_param(int)));
    connect(sl_brightness, SIGNAL(valueChanged(int)), this, SLOT(set_camera_param(int)));
    connect(sl_exposure, SIGNAL(valueChanged(int)), this, SLOT(set_camera_param(int)));
    connect(markView, SIGNAL(left_press(int,int)), this, SLOT(focus_point_select(int,int)));
    connect(bt_loadImage, SIGNAL(clicked()), this,SLOT(load_image()));
    connect(bt_savePhoto, SIGNAL(clicked()), this, SLOT(save_image()));
    connect(cb_autoBL,SIGNAL(stateChanged(int)),this,SLOT(auto_brightness(int)));
}


void MarkWidget::adjust_page_init(){
     char buf[16];
     sprintf(buf, "%8.3lf", param.referenceX);
     lb_holeDetectX->setText(buf);
     sprintf(buf, "%8.3lf", param.referenceY);
     lb_holeDetectY->setText(buf);
     sprintf(buf, "%8.3f", param.pickRelx);
     lb_ccdOffsetX->setText(buf);
     sprintf(buf, "%8.3f", param.pickRely);
     lb_ccdOffsetY->setText(buf);
     sprintf(buf, "%8.3f", param.glueRelx);
     lb_glueOffsetX->setText(buf);
     sprintf(buf, "%8.3f", param.glueRely);
     lb_glueOffsetY->setText(buf);

     for(int i=0;i<4;i++)
         connect(qtObjects.bt_adjustPos[i], SIGNAL(clicked()), this, SLOT(adjPos_pressed()));
     connect(bt_adjust, SIGNAL(clicked()), this, SLOT(adjust_pressed()));
     connect(bt_adjustClear, SIGNAL(clicked()),this, SLOT(adjust_clear_pressed()));
     connect(bt_inputHole, SIGNAL(clicked()),this, SLOT(input_hole_pos_pressed()));
     connect(bt_detectHole,SIGNAL(clicked()), this, SLOT(detect_hole_presssed()));
     connect(bt_gluePos,SIGNAL(clicked()), this, SLOT(input_glue_pos_pressed()));
}


void MarkWidget::mark_view_update(){

    static int autoIndex=-1;
    static int autoScanHoleIndex = -1;
    ImageCapture* currentCapture = NULL;

    currentCapture=capture;

    if(emcStatus.mode!=EMC_TASK_MODE_AUTO){
        if(currentCapture){
            currentCapture->get_image(srcImage);
        }
        markView->receive_image(srcImage);
        if(tabWidget->currentIndex()==2)
            markView->set_focus_box(true, focusAidView->get_focus_area());
        else
            markView->set_focus_box(false);
        autoIndex=-1;
        autoScanHoleIndex = -1;
    }
    else{
        if(autoIndex!=diamondPosition.scanIndex){
            autoIndex=diamondPosition.scanIndex;
            markView->receive_image(srcImage);
            markView->set_diamond_pos(diamondCirclesDetecter->get_positions(), diamondCirclesDetecter->radious());
            list<Point> empty;
            markView->set_hole_pos(empty, 0);
            markView->set_search_frame(prjManage.get_searcRectD());
        }
        if(autoScanHoleIndex != diamondPosition.scanHoleIndex){
            autoScanHoleIndex = diamondPosition.scanHoleIndex;
            markView->receive_image(srcImage);
            markView->set_hole_pos(watchCircleDetecter->get_positions(),watchCircleDetecter->radious());
            list<Point> empty;
            markView->set_diamond_pos(empty, 0);
            markView->set_search_frame(prjManage.get_searcRectW());
        }
       markView->set_focus_box(false);
    }


    //display camer status
    if(currentCapture==NULL)
       markView->set_ccd_status(MarkView::without_ccd);
    else {
        static int failDelay=0;
        if(currentCapture->camera_ok()){
            if(failDelay>0)
                failDelay--;
            else
                markView->set_ccd_status(MarkView::single_ccd);
        }
        else{
            markView->set_ccd_status(MarkView::comincate_fail);
            failDelay=10;
        }
    }

    //markView->set_pass(markKey.isPassed());

    markView->update();
}

void MarkWidget::view_update(){

    mark_view_update();

    if(tabWidget->currentIndex()==2){
        focusAidView->receive_image(srcImage);
        focusAidView->update();
    }

    //pattern image update

    if(diamondCirclesDetecter->pattern_is_new()){
        const IplImage* pattern=diamondCirclesDetecter->get_pattern();
        if(pattern){
            patternView->receive_image(pattern);
            patternView->update();
        }
    }

    if(watchCircleDetecter->pattern_is_new()){
        const IplImage* pattern = watchCircleDetecter->get_pattern();
        if(pattern){
            holePatternView->receive_image(pattern);
            holePatternView->update();
        }
    }
}

void MarkWidget::slow_cycle(){

#ifdef WITH_EMC
    emcStatus.update();
    emit(update_emc_status(emcStatus));
    if(emcStatus.stopToManual && emcStatus.hasStop
            && emcStatus.programStaut==EMC_TASK_INTERP_IDLE){
        emc_mode(NULL, EMC_TASK_MODE_MANUAL);
        emcStatus.stopToManual=false;
    }
#endif

    if(bt_detect0->isChecked()){

        diamondCirclesDetecter->detect(srcImage, &prjManage.get_searcRectD());
        markView->set_diamond_pos(diamondCirclesDetecter->get_positions(), diamondCirclesDetecter->radious());
    }

    if(bt_detectHoles->isChecked()){
        watchCircleDetecter->detect(srcImage, &prjManage.get_searcRectW());
        markView->set_hole_pos(watchCircleDetecter->get_positions(),watchCircleDetecter->radious());
    }

    infor.runTime = QString(emcStatus.prtManager.get_run_time_string().c_str());
    infor.diamondNum = diamondPosition.dimamondPos.size();
    infor.watchPosNum = posRecorder->get_pos_num();
    infor.holePosNum = (posRecorder->holesPosVec).size();
    emit(update_infor(infor));

    cv_cmd_cycle();
}

void MarkWidget::auto_detect_diamond(){

    diamondCirclesDetecter->detect(srcImage, &prjManage.get_searcRectD());
    const list<Point>& imgPos =diamondCirclesDetecter->get_positions();
    diamondPosition.scanIndex++;

    list<Point>::const_iterator it;
    Point pos;
    Vector2 vm(emcStatus.cmdAxis[0], emcStatus.cmdAxis[1]);
    for(it=imgPos.begin();it!=imgPos.end();it++){
        pos=transfMatrix->transform(it->x(), it->y());
        pos.move(vm);
        diamondPosition.dimamondPos.push_back(pos);
    }
    markView->set_diamond_sum((int)diamondPosition.dimamondPos.size());
}

void MarkWidget::auto_detect_watch(){
     diamondPosition.scanHoleIndex++;
     unsigned int markIndex = posRecorder->get_mark_index();
     const Position* position = posRecorder->get_position(markIndex-1);
     CvRect roi = position->get_search_cv_area();
     prjManage.set_searcRectW(roi);
     watchCircleDetecter->detect(srcImage, &prjManage.get_searcRectW());
     const list<Point>& holesPos = watchCircleDetecter->get_positions();

     list<Point>::const_iterator it;
     Point pos;
     Vector2 vm(emcStatus.cmdAxis[0], emcStatus.cmdAxis[1]);
     for(it=holesPos.begin();it!=holesPos.end();it++){
         pos=transfMatrix->transform(it->x(), it->y());
         pos.move(vm);
         Position hole(pos.x(),pos.y(),emcStatus.cmdAxis[2],emcStatus.cmdAxis[3],emcStatus.cmdAxis[4]);
         posRecorder->holesPosVec.push_back(hole);
     }
}


void MarkWidget::clear_diamond_pos(){
    diamondPosition.dimamondPos.clear();
    diamondPosition.scanIndex=0;
    markView->set_diamond_pos(diamondPosition.dimamondPos, diamondCirclesDetecter->radious());
    markView->set_diamond_sum(0);
}

void MarkWidget::clear_hole_pos()
{
    posRecorder->holesPosVec.clear();
    diamondPosition.scanHoleIndex = 0;
    list<Point> empty;
    markView->set_hole_pos(empty,0);
}

void MarkWidget::cv_cmd_cycle()
{
    MarkHalPins* halpins=halData->halpins;
    hal_s32_t cmd;
    static enum{
        MARK_IDLE,
        WAIT_STOP,
        WAIT_FRAME,
        MARK_DETECT
    }state=MARK_IDLE;

    //ready
    if(*halpins->readyIndex!=0){
        if(*halpins->readyIndex==1)
            ready_for_diamond_scan();
        else if(*halpins->readyIndex==2)
            ready_for_watch_scan();

        *halpins->readyIndex=0;
    }

    //detect
    cmd=*halpins->cvCmd;
    switch(state)
    {
    case MARK_IDLE:
        if(cmd)
            state=WAIT_STOP;
        break;

    case WAIT_STOP:
        if(emcStatus.hasStop)
            state=WAIT_FRAME;
        break;

    case WAIT_FRAME:
        state=MARK_DETECT;
        break;

    case MARK_DETECT:
        if(cmd==1){
            if(capture)
                capture->get_image(srcImage);
            *halpins->cvCmd=0;
            auto_detect_diamond();
            infor.diamondNum = diamondPosition.dimamondPos.size();
        }
        else if(cmd==2){
            if(capture)
                capture->get_image(srcImage);
            *halpins->cvCmd=0;
            posRecorder->incr_mark_index(1);
            auto_detect_watch();
            infor.holePosIndex = 0;
            infor.gluePosIndex = 0;

            if(posRecorder->get_mark_index()==posRecorder->get_pos_num()){

                posRecorder->finish_record_hole_pos();                
                posRecorder->set_mark_index(0);
            }

            markView->set_hole_pos(watchCircleDetecter->get_positions(), watchCircleDetecter->radious());
        }
        else
            *halpins->cvCmd=0;
        state=MARK_IDLE;
        break;

    default:
        state=MARK_IDLE;
    }
}

void MarkWidget::pickup_one_diamond(const Point &diamondPos, double pickupZ)
{
    halData->set_axis_pos(0, diamondPos.x()-param.pickup_offset_x());
    halData->set_axis_pos(1, diamondPos.y()-param.pickup_offset_y());
    halData->set_axis_pos(2, pickupZ);
}

void MarkWidget::set_one_glue(const Position &holePos, double setGlueZ)
{
    halData->set_axis_pos(0, holePos.get_value(0)-param.glue_offset_x());
    halData->set_axis_pos(1, holePos.get_value(1)-param.glue_offset_y());
    halData->set_axis_pos(2, setGlueZ);
    halData->set_axis_pos(3, holePos.get_value(3));
    halData->set_axis_pos(4, holePos.get_value(4));
}

void MarkWidget::set_one_diamond(const Position &holePos, double setDiamondZ)
{
    halData->set_axis_pos(0, holePos.get_value(0)-param.pickup_offset_x());
    halData->set_axis_pos(1, holePos.get_value(1)-param.pickup_offset_y());
    halData->set_axis_pos(2, setDiamondZ);
    halData->set_axis_pos(3, holePos.get_value(3));
    halData->set_axis_pos(4, holePos.get_value(4));
}

void MarkWidget::scan_one_pos_of_watch(const Position *watchPos)
{
    if(watchPos){
        int axis_num = 5;
        for(int i=0; i<axis_num; i++){
            halData->set_axis_pos(i, watchPos->get_value(i));
        }
    }
}

void MarkWidget::fast_react_cycle(){
     MarkHalPins* halpins=halData->halpins;
     int posCmd = *halpins->posCmd;
     *halpins->posCmd = 0;
     if(posCmd == 1){ //pickup diamond
        Point pos= *diamondPosition.dimamondPos.begin();
        pickup_one_diamond(pos, prjManage.get_pickupZD());
    }
    else if(posCmd == 2){ //scan watch
        const Position* watchPos = posRecorder->get_position(posRecorder->get_current_index());
        scan_one_pos_of_watch(watchPos);
        posRecorder->incr_current_index(1);
    }
     else if(posCmd == 3){ //set glue
         unsigned int index = posRecorder->get_glue_hole_index();
         if(index<(posRecorder->holesPosVec).size()){
             const Position& pos = (posRecorder->holesPosVec)[index];
             set_one_glue(pos, prjManage.get_glueZPos());
         }
         posRecorder->incr_glue_hole_index(1);
     }
     else if(posCmd == 4){ //set diamond
         unsigned int index = posRecorder->get_hole_index();
         if(index<(posRecorder->holesPosVec).size()){
             const Position& pos = (posRecorder->holesPosVec)[index];
             set_one_diamond(pos, prjManage.get_setDiamondZPos());
         }
         posRecorder->incr_hole_index(1);
     }

     //当有一个步骤完成时，更新相应的信息
     int reachCmd = *halpins->reachCmd;
     *halpins->reachCmd = 0;
     switch(reachCmd){
     case 1:
         diamondPosition.dimamondPos.pop_front();
         markView->set_diamond_sum((int)diamondPosition.dimamondPos.size());
         break;
     case 2:
         infor.watchPosIndex++;
         break;
     case 3:
         infor.gluePosIndex++;
         break;
     case 4:
         infor.holePosIndex++;
         break;
     }

    //更新IO按钮的状态
    for(int i = 0; i<halData->io_num; i++)
    {
        if(halData->pin_is_valid(i))
            infor.ioState[i] = true;
        else
            infor.ioState[i] = false;
    }

    if(*halpins->start)
        infor.start = true;
    else
        infor.start = false;

    if(*halpins->stop){
        infor.stop = true;
        *halpins->start = 0;
        *halpins->stop = 0;
    }
    else
        infor.stop = false;

    if(posRecorder->get_current_index()<=posRecorder->get_pos_num())
        *halpins->watchPosValid = 1;
    else{
        *halpins->watchPosValid = 0;
        if(!infor.endScanWatch)
            infor.endScanWatch = true;
    }

    if(diamondPosition.dimamondPos.size())
        *halpins->posValid=1;   
    else
        *halpins->posValid=0;  

    if(posRecorder->get_hole_index()<=(posRecorder->holesPosVec).size())
        *halpins->watchHoleValid = 1;
    else{
        *halpins->watchHoleValid = 0;
        if(!infor.endAutoRun){
            infor.endAutoRun = true;
            //
            *halpins->start = 0;
        }
        if(!infor.endSetDiamond)
            infor.endSetDiamond = true;
        //*halpins->start = 0;

    }
    if(posRecorder->get_glue_hole_index()<=(posRecorder->holesPosVec).size())
        *halpins->glueHoleValid = 1;
    else{
        *halpins->glueHoleValid = 0;
        if(!infor.endSetGLue)
            infor.endSetGLue = true;
    }
}

void MarkWidget::ready_for_diamond_scan(){
    clear_diamond_pos();
    sp_ADLevle->setValue(prjManage.get_adlD());
    sl_contrast->setValue(prjManage.get_contrastD());
    sl_brightness->setValue(prjManage.get_brightnessD());
    sl_exposure->setValue(prjManage.get_exposureD());
}

void MarkWidget::ready_for_watch_scan()
{
    clear_hole_pos();
    sp_ADLevle->setValue(prjManage.get_adlW());
    sl_contrast->setValue(prjManage.get_contrastW());
    sl_brightness->setValue(prjManage.get_brightnessW());
    sl_exposure->setValue(prjManage.get_exposureW());
}

void MarkWidget::select_pattern_toggled(bool checked){
    RectangleFrame frame;
    QPushButton* button=qobject_cast<QPushButton*>(sender());
    if(button==tb_selectPattern0 || button == tb_selectHolePattern){
        if(checked)
            markView->set_default_pattern_frame();
        else{
            frame=markView->get_pattern_frame();
            Point lt=frame.get_top_left();
            int pw=frame.get_width()+0.5;
            int ph=frame.get_height()+0.5;
            CvRect rect=cvRect(lt.x()+0.5-10, lt.y()+0.5-10, pw+20, ph+20);
            if(button == tb_selectPattern0){
                diamondCirclesDetecter->set_pattern(srcImage, &rect);
                prjManage.save_diamond_pattern(diamondCirclesDetecter->get_pattern());
            }
            else{
                CvRect tmpRect = cvRect(lt.x()-10+0.5,lt.y()-10+0.5, pw+20, ph+20);
                watchCircleDetecter->set_pattern(srcImage,&tmpRect);
                prjManage.save_watch_pattern(watchCircleDetecter->get_pattern());
            }
        }
    }

}

void MarkWidget::search_area_toggled(bool checked){
    QPushButton* button=qobject_cast<QPushButton*>(sender());
    if(button==tb_searchArea0 || button == tb_searchHoleArea){
        if(checked)
            markView->set_default_search_frame();
        else{
            RectangleFrame frame= markView->get_search_frame();
            Point lt=frame.get_top_left();
            int pw=frame.get_width()+0.5;
            int ph=frame.get_height()+0.5;
            CvRect searchArea;
            if(button == tb_searchArea0 ){
                searchArea = cvRect(lt.x()+0.5, lt.y()+0.5, pw, ph);
                prjManage.set_searcRectD(searchArea);
                prjManage.save_diamond_search_area();
            }
            else{
                searchArea = cvRect(lt.x()+0.5, lt.y()+0.5, pw, ph);
                prjManage.set_searcRectW(searchArea);
                prjManage.save_watch_search_area();
            }
        }
    }
}

void MarkWidget::diamond_test_toggled(bool checked){
    QPushButton* button=qobject_cast<QPushButton*>(sender());
    if(button == bt_detect0){
        if(checked){
            markView->set_search_frame(prjManage.get_searcRectD());
            prjManage.save_diamond_camera_param(param.camADL, param.camBL, param.camGain, param.camExposure);
        }
        else{
            list<Point> empty;
            markView->set_diamond_pos(empty, 0);
        }
    }
    else if(button == bt_detectHoles){
        if(checked){
            markView->set_search_frame(prjManage.get_searcRectW());
            prjManage.save_watch_camera_param(param.camADL, param.camBL, param.camGain, param.camExposure);
        }
        else{
            list<Point> empty;
            markView->set_hole_pos(empty, 0);
        }
    }
}


void MarkWidget::set_scan_beginning(){
    char buf[256];
    int i;
    for(i=0;i<3;i++){
        prjManage.set_scanStartPos(i,emcStatus.cmdAxis[i]);
    }
    double dx=prjManage.get_scanEndPos(0)-prjManage.get_scanStartPos(0);
    double dy=prjManage.get_scanEndPos(1)-prjManage.get_scanStartPos(1);
    double signx = dx>0?1:-1;
    double signy = dy>0?1:-1;
    prjManage.set_scanColDis(prjManage.get_searcRectD().width*param.kxx*signx);
    prjManage.set_scanRowDis(prjManage.get_searcRectD().height*param.kyy*signy);
    prjManage.set_scanColNum(dx/prjManage.get_scanColDis()+2);
    prjManage.set_scanRowNum(dy/prjManage.get_scanRowDis()+2);
#ifdef WITH_EMC
    emc_mode(NULL, EMC_TASK_MODE_MDI);
    sprintf(buf, "#%d=%.3f #%d=%.3f #%d=%.3f #%d=%d #%d=%d #%d=%.3f #%d=%.3f",
            ProjectManage::SCAN_X0,prjManage.get_scanStartPos(0),
            ProjectManage::SCAN_Y0,prjManage.get_scanStartPos(1),
            ProjectManage::SCAN_Z,prjManage.get_scanStartPos(2),
            ProjectManage::SCAN_ROW_NUM, prjManage.get_scanRowNum(),
            ProjectManage::SCAN_COL_NUM, prjManage.get_scanColNum(),
            ProjectManage::SCAN_ROW_DIS, prjManage.get_scanRowDis(),
            ProjectManage::SCAN_COL_DIS, prjManage.get_scanColDis());
    emc_mdi(buf);
    emc_mode(NULL, EMC_TASK_MODE_MANUAL);
#endif
    sprintf(buf, "%8.3f", prjManage.get_scanStartPos(0));
    lb_scanX0->setText(QString::fromUtf8(buf));
    sprintf(buf, "%8.3f", prjManage.get_scanStartPos(1));
    lb_scanY0->setText(QString::fromUtf8(buf));
    sprintf(buf, "%8.3f", prjManage.get_scanStartPos(2));
    lb_scanZ0->setText(QString::fromUtf8(buf));
    prjManage.save_scan_param();
}


void MarkWidget::back_scan_beginning(){
    char buf[64];
#ifdef WITH_EMC
    emc_mode(NULL, EMC_TASK_MODE_MDI);
    emc_mdi("g0 g53 z0");
    sprintf(buf, "g0 g53 x%f y%f", prjManage.get_scanStartPos(0), prjManage.get_scanStartPos(1));
    emc_mdi(buf);
    sprintf(buf, "g1 g53 z%f f3000", prjManage.get_scanStartPos(2));
    emc_mdi(buf);
    emcStatus.stopToManual=true;
#endif
}


void MarkWidget::set_scan_end(){
    char buf[256];
    int i;
    for(i=0;i<3;i++)
        prjManage.set_scanEndPos(i,emcStatus.cmdAxis[i]);
    double dx=prjManage.get_scanEndPos(0)-prjManage.get_scanStartPos(0);
    double dy=prjManage.get_scanEndPos(1)-prjManage.get_scanStartPos(1);
    double signx = dx>0?1:-1;
    double signy = dy>0?1:-1;
    prjManage.set_scanColDis(prjManage.get_searcRectD().width*param.kxx*signx);
    prjManage.set_scanRowDis(prjManage.get_searcRectD().height*param.kyy*signy);
    prjManage.set_scanColNum(dx/prjManage.get_scanColDis()+2);
    prjManage.set_scanRowNum(dy/prjManage.get_scanRowDis()+2);
#ifdef WITH_EMC
    emc_mode(NULL, EMC_TASK_MODE_MDI);
    sprintf(buf, "#%d=%d #%d=%d #%d=%.3f #%d=%.3f",
            ProjectManage::SCAN_ROW_NUM, prjManage.get_scanRowNum(),
            ProjectManage::SCAN_COL_NUM, prjManage.get_scanColNum(),
            ProjectManage::SCAN_ROW_DIS, prjManage.get_scanRowDis(),
            ProjectManage::SCAN_COL_DIS, prjManage.get_scanColDis());
    emc_mdi(buf);
    emc_mode(NULL, EMC_TASK_MODE_MANUAL);
#endif
    sprintf(buf, "%8.3f", prjManage.get_scanEndPos(0));
    lb_scanX1->setText(QString::fromUtf8(buf));
    sprintf(buf, "%8.3f", prjManage.get_scanEndPos(1));
    lb_scanY1->setText(QString::fromUtf8(buf));
    sprintf(buf, "%8.3f", prjManage.get_scanEndPos(2));
    lb_scanZ1->setText(QString::fromUtf8(buf));
    prjManage.save_scan_param();
}

void MarkWidget::back_scan_end(){
    char buf[64];
#ifdef WITH_EMC
    emc_mode(NULL, EMC_TASK_MODE_MDI);
    emc_mdi("g0 g53 z0");
    sprintf(buf, "g0 g53 x%f y%f", prjManage.get_scanEndPos(0), prjManage.get_scanEndPos(1));
    emc_mdi(buf);
    sprintf(buf, "g1 g53 z%f f3000", prjManage.get_scanEndPos(2));
    emc_mdi(buf);
    emcStatus.stopToManual=true;
#endif
}


void MarkWidget::scan_test(){
#ifdef WITH_EMC
    //printf("scan_test\n");
    emc_mode(NULL, EMC_TASK_MODE_AUTO);
    emc_open("/home/u/cnc/configs/ppmc/o_nc/scan.ngc");
    emc_wait("done");
    emc_run(0);
    emcStatus.stopToManual=true;
#endif
}



void MarkWidget::pickup_first(){
    char buf[128];
    if(diamondPosition.dimamondPos.size()==0)
        return;
    diamondPosition.pickupIterator=diamondPosition.dimamondPos.begin();
    Point pos=*diamondPosition.pickupIterator;
#ifdef WITH_EMC
    emc_mode(NULL,EMC_TASK_MODE_MDI);
    emc_mdi("g0 g53 z0");
    sprintf(buf, "g0 g53 x%f y%f", pos.x()-param.pickup_offset_x(), pos.y()-param.pickup_offset_y());
    emc_mdi(buf);
    sprintf(buf, "g1 g53 z%f f3000", prjManage.get_pickupZD());
    emc_mdi(buf);
    emcStatus.stopToManual=true;
#endif
}

void MarkWidget::pickup_next(){
    char buf[128];
    if(diamondPosition.dimamondPos.size()==0)
        return;
    diamondPosition.pickupIterator++;
    if(diamondPosition.pickupIterator==diamondPosition.dimamondPos.end()){
       diamondPosition.pickupIterator=diamondPosition.dimamondPos.begin();
    }
    Point pos= *diamondPosition.pickupIterator;
#ifdef WITH_EMC
    emc_mode(NULL,EMC_TASK_MODE_MDI);
    sprintf(buf, "g0 g53 z%f", prjManage.get_pickupZD()+5);
    emc_mdi(buf);
    sprintf(buf, "g0 g53 x%f y%f", pos.x()-param.pickup_offset_x(), pos.y()-param.pickup_offset_y());
    emc_mdi(buf);
    sprintf(buf, "g1 g53 z%f f3000", prjManage.get_pickupZD());
    emc_mdi(buf);
    emcStatus.stopToManual=true;
#endif
}

void MarkWidget::pickup_all(){
#ifdef WITH_EMC
    emc_mode(NULL,EMC_TASK_MODE_AUTO);
    emc_open("/home/u/cnc/configs/ppmc/o_nc/pickup.ngc");
    emc_wait("done");
    emc_run(0);
    emcStatus.stopToManual=true;
#endif
}

void MarkWidget::on_bt_clearDiamond_clicked()
{
    clear_diamond_pos();
}

void MarkWidget::choose_pattern_shap( )
{
    QRadioButton* radio = qobject_cast<QRadioButton*>(sender());
    if(radio == rb_circle)
        markView->set_show_circle(true);
    else
        markView->set_show_circle(false);
}

bool compare_points(const Point& p1, const Point& p2)
{
    Vector2 vec1(p1.x(),p1.y());
    Vector2 vec2(p2.x(),p2.y());
    if(vec1.length()<=vec2.length())
        return true;
    else
        return false;
}

bool distance_within(const Point& p1, const Point& p2)
{
    Vector2 vec(p1,p2);
    return (vec.length()>1);
}


//void MarkWidget::record_diamond_pos()
//{
//    ofstream ofs;
//    list<Point> posList = diamondPosition.dimamondPos;
//    list<Point> tmpPosList;
//    list<double> tmpList;
//    list<Point>::iterator iter,iter1;
//    list<double>::iterator doubleIter;

//    posList.sort(compare_points);

//    for(iter = posList.begin(); iter!=posList.end();iter++){
//        iter1 = iter;
//        if(++iter1 != posList.end()){
//            Vector2 vec(*iter,*iter1);
//            tmpList.push_back(vec.length());
//        }
//        else
//            break;
//    }

//    ofs.open("/home/u/cnc/镶钻存档/test1/diamond.pos");

//    if(!ofs.is_open()){
//        cout<<"open file /home/u/cnc/镶钻存档/test1/diamond.pos fail."<<endl;
//        return;
//    }
//    ofs.precision(6);

//    for(iter = posList.begin(); iter != posList.end(); iter++){
//        Vector2 vec((*iter).x(),(*iter).y());
//        ofs<<(*iter).x()<<" "<<(*iter).y()<<" "<<vec.length()<<endl;
//    }
//    ofs<<endl<<endl;
//    for(doubleIter = tmpList.begin(); doubleIter != tmpList.end(); doubleIter++){
//        ofs<<*doubleIter<<endl;
//    }

//    ofs<<endl<<endl;
//    tmpPosList = posList;

//    for(iter = posList.begin(); iter!=posList.end(); iter++){
//        if(std::binary_search(posList.begin(),posList.end(), *iter,distance_within))
//            tmpPosList.remove(*iter);
//    }

//    for(iter = tmpPosList.begin(); iter != tmpPosList.end(); iter++){
//        Point imgP = transfMatrix->inv_transform(*iter);
//        ofs<<imgP.x()<<" "<<imgP.y()<<endl;
//    }

//    ofs.close();
//}

void MarkWidget::change_angle(){
    double sign=1;
    if(bt_rotateBackward==qobject_cast<QPushButton*>(sender())){
        sign=-1;
    }
    double val=fabs(le_rotateDeg->text().toDouble());
    if(fabs(val-param.degInc)>0.0005){
        param.degInc=val;
        write_profile_double("MARK", "DEGREE_INCREASE", val, EMC_INIFILE);
    }
    val*=sign;
    emc_jog_incr(4,10,val);
}


//设置取钻高度
void MarkWidget::set_pickup_diamnod_z(double value){
    char buf[32];
    if(fabs(value - 0)<0.00001) //if value == 0
        prjManage.set_pickupZD(emcStatus.cmdAxis[2]);
    else
        prjManage.set_pickupZD(value);
    write_profile_double("DIAMOND", "PICKUP_Z", prjManage.get_pickupZD(), prjManage.ini_file());
#ifdef WITH_EMC
    emc_mode(NULL,EMC_TASK_MODE_MDI);
    sprintf(buf, "#%d=%f", ProjectManage::PICKUP_Z, prjManage.get_pickupZD());
    emc_mdi(buf);
    emc_mode(NULL,EMC_TASK_MODE_MANUAL);
#endif
}

//设置滴胶高度
void MarkWidget::set_glue_z_pos(double value)
{
    char buf[32];
    if(fabs(value - 0)<0.00001)
        prjManage.set_glueZPos(emcStatus.cmdAxis[2]);
    else
        prjManage.set_glueZPos(value);
    write_profile_double("WATCH", "GLUE_Z_POS", prjManage.get_glueZPos(), prjManage.ini_file());
#ifdef WITH_EMC
    emc_mode(NULL,EMC_TASK_MODE_MDI);
    sprintf(buf, "#%d=%f", ProjectManage::SET_GLUE_Z, prjManage.get_glueZPos());
    emc_mdi(buf);
    emc_mode(NULL,EMC_TASK_MODE_MANUAL);
#endif

}
//设置镶钻高度
void MarkWidget::set_setdiamond_z_pos(double value)
{
    char buf[32];
    if(fabs(value - 0)<0.00001)
        prjManage.set_setDiamondZPos(emcStatus.cmdAxis[2]);
    else
        prjManage.set_setDiamondZPos(value);
    write_profile_double("WATCH", "SET_DIAMOND_Z_POS", prjManage.get_setDiamondZPos(), prjManage.ini_file());

#ifdef WITH_EMC
    emc_mode(NULL,EMC_TASK_MODE_MDI);
    sprintf(buf, "#%d=%f", ProjectManage::SETDIAMOND_Z, prjManage.get_setDiamondZPos());
    emc_mdi(buf);
    emc_mode(NULL,EMC_TASK_MODE_MANUAL);
#endif
}

//设置各种延时
void MarkWidget::set_time(int index, int varNum, double value)
{
    char buf[32];
    string times[4] = {"SET_GLUE_TIME", "AFTER_GLUE_TIME","GET_DIAMOND_TIME","SET_DIAMOND_TIME"};
    switch(index){
    case 0:
        prjManage.set_glueTime(value);
        break;
    case 1:
        prjManage.set_afterGlueTime(value);
        break;
    case 2:
        prjManage.set_getDiamondTime(value);
        break;
    case 3:
        prjManage.set_setDiamondTime(value);
        break;
    }
    //printf("Time: %s,%f\n",times[index].c_str(),value);
    write_profile_double("TIME", times[index].c_str(), value, prjManage.ini_file());
#ifdef WITH_EMC
    emc_mode(NULL,EMC_TASK_MODE_MDI);
    sprintf(buf, "#%d=%f", varNum, value);
    emc_mdi(buf);
    emc_mode(NULL,EMC_TASK_MODE_MANUAL);
#endif
}

void MarkWidget::record_watch_pos()
{
    RectangleFrame searchArea = markView->get_search_frame();
    posRecorder->record_current_pos(emcStatus.cmdAxis,searchArea);
    infor.watchPosIndex = posRecorder->get_pos_num();
    //infor.watchPosNum = posRecorder->get_pos_num();
    bt_abandonCurrentPos->setEnabled(true);
    bt_abandonAllPos->setEnabled(true);
    bt_finishRecord->setEnabled(true);
    bt_autoRecord->setEnabled(true);
}

void MarkWidget::abandon_current_pos()
{
    int tmpIntNum;
    posRecorder->abandon_current_pos();
    tmpIntNum = posRecorder->get_pos_num();
    infor.watchPosIndex = posRecorder->get_pos_num();
    //infor.watchPosNum = posRecorder->get_pos_num();
    bt_abandonCurrentPos->setEnabled(false);
    if(tmpIntNum == 0){
        bt_abandonCurrentPos->setEnabled(false);
        bt_abandonAllPos->setEnabled(false);
        bt_finishRecord->setEnabled(false);
        bt_autoRecord->setEnabled(false);
    }
    else{
        bt_abandonCurrentPos->setEnabled(true);
        bt_abandonAllPos->setEnabled(true);
        bt_finishRecord->setEnabled(true);
        bt_autoRecord->setEnabled(true);
    }
}

void MarkWidget::abandon_all_pos()
{
    get_first_pos();
    posRecorder->abandon_all_pos();
    infor.watchPosIndex = posRecorder->get_pos_num();
    //infor.watchPosNum = posRecorder->get_pos_num();
    bt_abandonCurrentPos->setEnabled(false);
    bt_abandonAllPos->setEnabled(false);
    bt_finishRecord->setEnabled(false);
    bt_autoRecord->setEnabled(false);
    bt_firstPos->setEnabled(false);
    bt_nextPos->setEnabled(false);
    bt_camRun->setEnabled(false);
    bt_recordCamPos->setEnabled(true);
}

void MarkWidget::finish_record_watch_pos()
{
    posRecorder->finish_record_watch_pos();
    bt_abandonCurrentPos->setEnabled(false);
    bt_finishRecord->setEnabled(false);
    bt_recordCamPos->setEnabled(false);
    bt_firstPos->setEnabled(true);
    bt_nextPos->setEnabled(true);
    bt_camRun->setEnabled(true);
    bt_autoRecord->setEnabled(false);
    prjManage.save_diamond_camera_param(param.camADL, param.camBL, param.camGain, param.camExposure);
}

void MarkWidget::auto_record_watch_pos()
{
    int num = sp_posNum->value();
    double deg = le_rotateDeg->text().toDouble();
    int direction = cb_direction->currentIndex(); //0:反向, 1: 正向
    assert(direction<=1);
    if(direction == 0)
        direction = -1;
    deg *= direction;
    posRecorder->auto_record_watch_pos(num,deg);

    bt_abandonCurrentPos->setEnabled(false);
    bt_finishRecord->setEnabled(false);
    bt_recordCamPos->setEnabled(false);
    bt_firstPos->setEnabled(true);
    bt_nextPos->setEnabled(true);
    bt_camRun->setEnabled(true);
    bt_autoRecord->setEnabled(false);

    prjManage.save_diamond_camera_param(param.camADL, param.camBL, param.camGain, param.camExposure);
}

void MarkWidget::get_first_pos()
{
#ifdef WITH_EMC
    char buf[128];
    const Position* pos = posRecorder->first_position();
    if(pos)
    {
        emc_mode(NULL,EMC_TASK_MODE_MDI);
        emc_mdi("g0 g53 z0");
        sprintf(buf,"g0 g53 x%.3f y%.3f",pos->get_value(0), pos->get_value(1));
        emc_mdi(buf);
        sprintf(buf, "g0 g53 x%.3f y%.3f z%.3f a%.3f b%.3f", pos->get_value(0), pos->get_value(1),
                pos->get_value(2),pos->get_value(3),pos->get_value(4));
        emc_mdi(buf);
        emcStatus.stopToManual=true;
        markView->set_search_frame(pos->get_search_cv_area());
    }
#endif
}

void MarkWidget::get_next_pos()
{
#ifdef WITH_EMC
    char buf[128];
    const Position* pos = posRecorder->next_position();
    if(pos)
    {
        emc_mode(NULL,EMC_TASK_MODE_MDI);
        sprintf(buf, "g0 g53 x%.3f y%.3f z%.3f a%.3f b%.3f", pos->get_value(0), pos->get_value(1),
                pos->get_value(2),pos->get_value(3),pos->get_value(4));
        emc_mdi(buf);
        emcStatus.stopToManual=true;
        markView->set_search_frame(pos->get_search_cv_area());
    }
#endif
}


void MarkWidget::cam_run()
{
#ifdef WITH_EMC    
    if(emcStatus.programStaut==EMC_TASK_INTERP_PAUSED){
        emc_resume();
    }
    else if(emcStatus.programStaut==EMC_TASK_INTERP_IDLE){
        posRecorder->set_current_index(0);
        posRecorder->clear_holes_pos();
        infor.watchPosIndex = 0;
        infor.endScanWatch = false;
        MarkHalPins* halpins=halData->halpins;
        *halpins->watchPosValid = 1;
        emc_mode(NULL,EMC_TASK_MODE_AUTO);
        emc_open("/home/u/cnc/configs/ppmc/o_nc/watch.ngc");
        emc_wait("done");
        emc_run(0);
        emcStatus.stopToManual=true;
    }
#endif
}

void MarkWidget::set_first_hole()
{    
    if((posRecorder->holesPosVec).size()<=0)
        return;
    char buf[128];
    posRecorder->holeIter = (posRecorder->holesPosVec).begin();
    const Position pos = *(posRecorder->holeIter);
    //if(diamondPosition.dimamondPos.size()==0){
    //    cout<<"没有钻石了！"<<endl;
    //    return;
    //}
    //Point diamondPos= *diamondPosition.dimamondPos.begin();
#ifdef WITH_EMC
    //if(pos)
    //{
        emc_mode(NULL,EMC_TASK_MODE_MDI);
        //sprintf(buf,"g0 g53 z%.3f",prjManage.sendZD);
        //emc_mdi(buf);
        //sprintf(buf,"g0 g53 x%.3f y%.3f",diamondPos.x(),diamondPos.y());
        //emc_mdi(buf);
        //sprintf(buf,"g0 g53 z%.3f",prjManage.pickupZD);
        //emc_mdi(buf);
        //sprintf(buf,"g0 g53 z%.3f",prjManage.sendZD);
        //emc_mdi(buf);

        emc_mdi("g0 g53 z0");
        sprintf(buf,"g0 g53 x%.3f y%.3f a%.3f b%.3f",pos.get_value(0)-param.pickup_offset_x(),
                pos.get_value(1)-param.pickup_offset_y(), pos.get_value(3),pos.get_value(4));
        emc_mdi(buf);
        sprintf(buf, "g0 g53 z%.3f", prjManage.get_setDiamondZPos());
        emc_mdi(buf);
        emcStatus.stopToManual=true;
        //diamondPosition.dimamondPos.pop_front();
    //}
#endif

}

void MarkWidget::set_next_hole()
{
    if((posRecorder->holesPosVec).size()<=0)
        return;
    char buf[128];
    posRecorder->holeIter++;
    if(posRecorder->holeIter == (posRecorder->holesPosVec).end())
        posRecorder->holeIter = (posRecorder->holesPosVec).begin();
    const Position pos = *(posRecorder->holeIter);
    //if(diamondPosition.dimamondPos.size()==0){
    //    cout<<"没有钻石了！"<<endl;
    //    return;
    //}
    //Point diamondPos= *diamondPosition.dimamondPos.begin();
#ifdef WITH_EMC
    //if(pos)
    //{
        emc_mode(NULL,EMC_TASK_MODE_MDI);
        //sprintf(buf,"g0 g53 z%.3f",prjManage.sendZD);
        //emc_mdi(buf);
        //sprintf(buf,"g0 g53 x%.3f y%.3f",diamondPos.x(),diamondPos.y());
        //emc_mdi(buf);
        //sprintf(buf,"g0 g53 z%.3f",prjManage.pickupZD);
        //emc_mdi(buf);
        //sprintf(buf,"g0 g53 z%.3f",prjManage.sendZD);
        //emc_mdi(buf);

        emc_mdi("g0 g53 z0");
        sprintf(buf,"g0 g53 x%.3f y%.3f a%.3f b%.3f",pos.get_value(0)-param.pickup_offset_x(),
                pos.get_value(1)-param.pickup_offset_y(), pos.get_value(3),pos.get_value(4));
        emc_mdi(buf);
        sprintf(buf, "g0 g53 z%.3f",  prjManage.get_setDiamondZPos());
        emc_mdi(buf);
        emcStatus.stopToManual=true;
        //diamondPosition.dimamondPos.pop_front();
    //}
#endif
}

//点胶
void MarkWidget::set_all_holes()
{
#ifdef WITH_EMC
    if(emcStatus.programStaut==EMC_TASK_INTERP_PAUSED){
        emc_resume();
    }
    else if(emcStatus.programStaut==EMC_TASK_INTERP_IDLE){
        if((posRecorder->holesPosVec).size()<=0)
            return;
        posRecorder->set_hole_index(0);
        posRecorder->set_glue_hole_index(0);
        infor.gluePosIndex = 0;
        infor.endSetGLue = false;
        MarkHalPins* halpins=halData->halpins;
        *halpins->watchHoleValid = 1;
        *halpins->glueHoleValid = 1;
        emc_mode(NULL,EMC_TASK_MODE_AUTO);
        emc_open("/home/u/cnc/configs/ppmc/o_nc/setglue.ngc");
        emc_wait("done");
        emc_run(0);
        emcStatus.stopToManual=true;
    }
#endif
}

//镶钻
void MarkWidget::set_diamond_in_all_holes()
{
#ifdef WITH_EMC
    if(emcStatus.programStaut==EMC_TASK_INTERP_PAUSED){
        emc_resume();
    }
    else if(emcStatus.programStaut==EMC_TASK_INTERP_IDLE){
        if((posRecorder->holesPosVec).size()==0)
            return;
        posRecorder->set_hole_index(0);
        infor.holePosIndex = 0;
        infor.endSetDiamond = false;
        MarkHalPins* halpins=halData->halpins;
        *halpins->watchHoleValid = 1;
        emc_mode(NULL,EMC_TASK_MODE_AUTO);
        emc_open("/home/u/cnc/configs/ppmc/o_nc/setdiamond.ngc");
        emc_wait("done");
        emc_run(0);
        emcStatus.stopToManual=true;
    }

#endif

}

//自动镶钻，没有钻石时会自动扫描料盘,需手动点胶和扫描表壳
void MarkWidget::half_auto_set_diamond()
{
#ifdef WITH_EMC
    if(emcStatus.programStaut==EMC_TASK_INTERP_PAUSED){
        emc_resume();
    }
    else if(emcStatus.programStaut == EMC_TASK_INTERP_IDLE){
        if((posRecorder->holesPosVec).size()==0)
            return;
        posRecorder->set_hole_index(0);
        posRecorder->set_glue_hole_index(0);
        infor.holePosIndex = 0;
        infor.gluePosIndex = 0;
        infor.endAutoRun = false;

        MarkHalPins* halpins=halData->halpins;
        *halpins->glueHoleValid = 1;
        emc_mode(NULL,EMC_TASK_MODE_AUTO);
        emc_open("/home/u/cnc/configs/ppmc/o_nc/autorun.ngc");
        emc_wait("done");
        emc_run(0);
        emcStatus.stopToManual=true;
    }
#endif
}

//所有的步骤都自动
void MarkWidget::auto_set_diamond()
{
#ifdef WITH_EMC
    if(emcStatus.programStaut==EMC_TASK_INTERP_PAUSED){
        emc_resume();
    }
    else if(emcStatus.programStaut == EMC_TASK_INTERP_IDLE){
        posRecorder->set_current_index(0);        
        posRecorder->set_glue_hole_index(0);
        posRecorder->set_hole_index(0);
        infor.watchPosIndex = 0;
        infor.holePosIndex = 0;
        infor.gluePosIndex = 0;
        infor.endAutoRun = false;

        MarkHalPins* halpins=halData->halpins;
        *halpins->watchPosValid = 1;
        *halpins->watchHoleValid = 1;
        emc_mode(NULL,EMC_TASK_MODE_AUTO);
        emc_open("/home/u/cnc/configs/ppmc/o_nc/autorun2.ngc");
        emc_wait("done");
        emc_run(0);
        emcStatus.stopToManual=true;
    }
#endif
}


//void MarkWidget::cancel_area_select(){
//    markView->cancel_area_select();
//    QPushButton* tb_selectPattern=NULL;
//    QPushButton* tb_searchArea=NULL;
//    for(int i=0;i<4;i++){
//        if(qtObjects.tb_selectPattern[i]->isChecked())
//            tb_selectPattern=qtObjects.tb_selectPattern[i];
//        if(qtObjects.tb_searchArea[i]->isChecked())
//            tb_searchArea=qtObjects.tb_searchArea[i];
//        qtObjects.tb_selectPattern[i]->setEnabled( param.validPoint[i]);
//        qtObjects.tb_searchArea[i]->setEnabled( param.validPoint[i]);
//    }
//    if(param.validPointBM==0x0A){
//        qtObjects.tb_selectPattern[1]->setEnabled(true);
//        qtObjects.tb_searchArea[1]->setEnabled(true);
//    }
//    if(tb_selectPattern){
//        disconnect(tb_selectPattern, SIGNAL(toggled(bool)), this, SLOT(select_pattern_toggled(bool)));
//        tb_selectPattern->setChecked(false);
//        connect(tb_selectPattern, SIGNAL(toggled(bool)), this, SLOT(select_pattern_toggled(bool)));
//    }
//    if(tb_searchArea){
//        disconnect(tb_searchArea, SIGNAL(toggled(bool)), this, SLOT(search_area_toggled(bool)));
//        tb_searchArea->setChecked(false);
//        connect(tb_searchArea, SIGNAL(toggled(bool)), this, SLOT(search_area_toggled(bool)));
//    }
//}


void MarkWidget::load_project(const char *projectName)
{
    prjManage.change_current_project(projectName);
    if(posRecorder){
        posRecorder->change_project(prjManage.project_dir());
        init_information();
    }
    sprintf(param.projectName,"%s",projectName);
    write_profile_string("MARK", "CURRENT_PROJECT", projectName,EMC_INIFILE);
    lb_project->setText(QString::fromUtf8(param.projectName));
    infor.changeProject = true;
}


void MarkWidget::show_load_dialog(){
    loadRecordDialog->set_combo_box(param.projectName);
    if(loadRecordDialog->exec()==QDialog::Accepted){
       QString prjName= loadRecordDialog->get_project_name();
       if(prjName.size()>0){
           load_project(prjName.toUtf8().constData());
       }
    }
}


//void MarkWidget::show_adjust_intf(){
//    int r;
//    adjPSW->set_adjust_mode();
//    r=adjPSW->exec();
//    if(r==QDialog::Accepted){
//        if(adjPSW->get_enter_key()=="runf" ||adjPSW->get_enter_key()=="RUNF"){
//            tabWidget->addTab(adjTable,QString::fromUtf8("校正"));
//            tabWidget->setCurrentIndex(3);
//        }
//        else{
//            QMessageBox::warning(this,QString::fromUtf8("密码错误"),
//                                        QString::fromUtf8("校正密码错误，请重新输入"),
//                                        QMessageBox::Yes);
//        }
//    }
//}


//void MarkWidget::enter_license_key(){
//    bool pass=false;
//    static int errorTimes=0;
//    if(errorTimes>=3){
//        QMessageBox::warning(this, QString::fromUtf8("警告"),
//                             QString::fromUtf8("多次错误输入\n, 请重启软件"));
//        return ;
//    }
//    //adjPSW->set_license_mode(markKey.get_sn());
//    if(adjPSW->exec()==QDialog::Accepted){
//        //pass=markKey.enter_key(adjPSW->get_enter_key().toUtf8().constData());
//        if(!pass){
//            errorTimes++;
//            QMessageBox::warning(this, QString::fromUtf8("警告"),
//                                 QString::fromUtf8("授权密码错误,\n 请重新输入"));
//        }
////        else
////            bt_license->hide();
//    }
//}


void MarkWidget::exit_adjust_intf(){
    tabWidget->setCurrentIndex(0);
    tabWidget->removeTab(3);
}

void MarkWidget::adjPos_pressed()
{
    int i, res=0;
    char buf[32];
    Point pos;

    //MarkResult* result=&markResult[0];

    for(i=0;i<4;i++){
        if(qtObjects.bt_adjustPos[i]==qobject_cast<QPushButton*>(sender()))
            break;
    }


    Vector2 vm(emcStatus.cmdAxis[0], emcStatus.cmdAxis[1]);
    if(rb_circle->isChecked()){
        const list<Point> positions=diamondCirclesDetecter->detect(srcImage,&prjManage.get_searcRectD());
        if(positions.size()>0){
            res=0;
            pos=*positions.begin();
            markView->set_diamond_pos(list<Point>(1, pos), diamondCirclesDetecter->radious());
            pos=transfMatrix->transform(pos.x(), pos.y());
            pos.move(vm);

        }
    }

    if(!res)
    {
        sprintf(buf, "%8.3lf", pos.x());
        qtObjects.lb_adjPosX[i]->setText(buf);
        sprintf(buf, "%8.3lf", pos.y());
        qtObjects.lb_adjPosY[i]->setText(buf);
        adjust.pos[i]=Point(emcStatus.cmdAxis[0], emcStatus.cmdAxis[1]);
        adjust.center[i]=Point(pos.x()-emcStatus.cmdAxis[0],
            pos.y()-emcStatus.cmdAxis[1]);
        adjust.posBitmap |= 1<<i;
    }
    else
    {
        qtObjects.lb_adjPosX[i]->setText(QString::fromUtf8("无效"));
        adjust.posBitmap &= ~(1<<i);
    }
}


void MarkWidget::adjust_pressed()
{
    if(adjust.posBitmap!=0x0f){
        QMessageBox::warning(this,QString::fromUtf8("警告"),
                                    QString::fromUtf8("数据不足，需要在4个\n不同位置识别同一点"),
                                    QMessageBox::Yes);
    }
    else{
        int r=QMessageBox::information(this,QString::fromUtf8("提示"),
                                    QString::fromUtf8("校正相机参数，并写入文件"),
                                    QMessageBox::Yes|QMessageBox::No);
        if(QMessageBox::Yes==r){
            mark_adjust_param();
        }
    }
}

void MarkWidget::adjust_clear_pressed(){
    adjust.posBitmap=0;
    int i;
    for(i=0;i<4;i++) {
        qtObjects.lb_adjPosX[i]->clear();
        qtObjects.lb_adjPosY[i]->clear();
    }
}



void MarkWidget::input_hole_pos_pressed(){
    double x,y;
    char buf[32];

    x=emcStatus.cmdAxis[0];
    y=emcStatus.cmdAxis[1];
    param.pickRelx=param.referenceX-x;
    param.pickRely=param.referenceY-y;
    sprintf(buf, "%8.3lf", x);
    lb_holeX->setText(buf);
    sprintf(buf, "%8.3lf", y);
    lb_holeY->setText(buf);  
    sprintf(buf, "%8.3lf", param.pickRelx);
    lb_ccdOffsetX->setText(buf);
    sprintf(buf, "%8.3lf", param.pickRely);
    lb_ccdOffsetY->setText(buf);

    write_profile_double( "MARK", "CAM_REL_SPINDLE_X", param.pickRelx, EMC_INIFILE);
    write_profile_double( "MARK", "CAM_REL_SPINDLE_Y", param.pickRely, EMC_INIFILE);

}

void MarkWidget::detect_hole_presssed(){
    int res=0;
    double cx=0,cy=0;
    char buf[32];
    Point pos;
    Vector2 vm(emcStatus.cmdAxis[0], emcStatus.cmdAxis[1]);
    if(rb_circle->isChecked()){
        const list<Point>& positions=diamondCirclesDetecter->detect(srcImage, &prjManage.get_searcRectD());
        if(positions.size()>0){
            res=0;
            pos=*diamondCirclesDetecter->get_positions().begin();
            markView->set_diamond_pos(list<Point>(1, pos),diamondCirclesDetecter->radious());
            pos=transfMatrix->transform(pos.x(), pos.y());
            pos.move(vm);
            cx=pos.x();
            cy=pos.y();
        }
    }

    if(!res){
        param.referenceX=cx;
        param.referenceY=cy;
        sprintf(buf, "%8.3lf", cx);
        lb_holeDetectX->setText(buf);
        sprintf(buf, "%8.3lf", cy);
        lb_holeDetectY->setText(buf);
        write_profile_double( "MARK", "REFERENCE_X", param.referenceX, EMC_INIFILE);
        write_profile_double( "MARK", "REFERENCE_Y", param.referenceY, EMC_INIFILE);
    }
    else{
        lb_holeDetectX->setText(QString::fromUtf8("无效"));
        lb_holeDetectY->setText(QString::fromUtf8("无效"));
    }
}

void MarkWidget::input_glue_pos_pressed(){
    char buf[32];
    double x=emcStatus.cmdAxis[0];
    double y=emcStatus.cmdAxis[1];
    param.glueRelx=param.referenceX-x;
    param.glueRely=param.referenceY-y;

    sprintf(buf, "%8.3lf", x);
    lb_glueX->setText(buf);
    sprintf(buf, "%8.3lf", y);
    lb_glueY->setText(buf);

    sprintf(buf, "%8.3lf", param.glueRelx);
    lb_glueOffsetX->setText(buf);
    sprintf(buf, "%8.3lf", param.glueRely);
    lb_glueOffsetY->setText(buf);

    write_profile_double( "MARK", "GLUE_REL_SPINDLE_X", param.glueRelx, EMC_INIFILE);
    write_profile_double( "MARK", "GLUE_REL_SPINDLE_Y", param.glueRely, EMC_INIFILE);
}

void MarkWidget::set_offset(int index, double value)
{
/*    switch(index){
        //Pickup X Offset
    case 0:
        param.pickupOffsetX = value;
        write_profile_double("MARK", "PICKUP_OFFSET_X", param.pickupOffsetX, EMC_INIFILE);
        break;
        //Pickup Y Offset
    case 1:
        param.pickupOffsetY = value;
        write_profile_double("MARK", "PICKUP_OFFSET_Y", param.pickupOffsetY, EMC_INIFILE);
        break;
        //Glue X Offset
    case 2:
        param.glueOffsetX = value;
        write_profile_double("MARK", "GLUE_OFFSET_X", param.glueOffsetX, EMC_INIFILE);
        break;
        //Glue Y Offset
    case 3:
        param.glueOffsetY = value;
        write_profile_double("MARK", "GLUE_OFFSET_Y", param.glueOffsetY, EMC_INIFILE);
        break;
    }
    */

    switch(index){
    case 0:
        prjManage.set_pickupOffsetX(value);
        param.pickupOffsetX = value;
        write_profile_double("OFFSET", "PICKUP_OFFSET_X", prjManage.get_pickupOffsetX(),
                             prjManage.ini_file());
        break;
    case 1:
        prjManage.set_pickupOffsetY(value);
        param.pickupOffsetY = value;
        write_profile_double("OFFSET", "PICKUP_OFFSET_Y", prjManage.get_pickupOffsetY(),
                             prjManage.ini_file());
        break;
    case 2:
        prjManage.set_glueOffsetX(value);
        param.glueOffsetX = value;
        write_profile_double("OFFSET", "GLUE_OFFSET_X", prjManage.get_glueOffsetX(),
                             prjManage.ini_file());
        break;
    case 3:
        prjManage.set_glueOffsetY(value);
        param.glueOffsetY = value;
        write_profile_double("OFFSET", "GLUE_OFFSET_Y", prjManage.get_glueOffsetY(),
                             prjManage.ini_file());
        break;
    }

}

void MarkWidget::set_diamond_detect_algorithm(int type)
{
    if(type == 0){
        diamondCirclesDetecter->setAlgorithmType(DiamondCircleDetecter::DT);
    }
    else if(type == 1){
        diamondCirclesDetecter->setAlgorithmType(DiamondCircleDetecter::HOUGH_CIRCLE);
    }
}

void MarkWidget::set_dt_threshold(int value)
{
    diamondCirclesDetecter->setDtThreshold(value);
}

int MarkWidget::get_dt_threshold()
{
    return diamondCirclesDetecter->getDtThreshold();
}

void MarkWidget::set_dt_pix_num_differ(int value)
{
    diamondCirclesDetecter->setDtPixNumDiffer(value);
}

int MarkWidget::get_dt_pix_num_differ()
{
    return diamondCirclesDetecter->getDtPixNumDiffer();
}

void MarkWidget::set_dt_search_region_width(double value)
{
    Point imgPoint;
    imgPoint = transfMatrix->inv_transform(value,0);
    int width = imgPoint.x()+0.5;
    diamondCirclesDetecter->setDtSearchRegionWidth(width);
}

double MarkWidget::get_dt_search_region_width()
{
    int width = diamondCirclesDetecter->getDtSearchRegionWidth();
    //printf("%d\n",width);
    double actual;
    actual = width*transfMatrix->kx();
    //printf("%f\n",actual);
    return actual;
}

void MarkWidget::set_distance_between_diamonds(double value)
{
    int width = value*transfMatrix->in_kx()+0.5;
    printf("Pix distance:%d\n",width);
    diamondCirclesDetecter->setDistanceBetweenDiamonds(width);
}

void MarkWidget::mark_adjust_param()
{
    char buf[256];
    double kxx, kxy, kyx, kyy;
    double qxx,qxy,qyx,qyy;
    double rxx, rxy, ryx, ryy;
    adjust.imageAdjust.clear();
    for(int i=0;i<4;i++)
        adjust.imageAdjust.add(adjust.center[i], adjust.pos[i]);
    adjust.imageAdjust.figure_up(qxx, qxy, qyx, qyy);

    kxx=param.kxx;
    kxy=param.kxy;
    kyx=param.kyx;
    kyy=param.kyy;

    rxx=qxx*kxx+qxy*kyx;
    rxy=qxx*kxy+qxy*kyy;
    ryx=qyx*kxx+qyy*kyx;
    ryy=qyx*kxy+qyy*kyy;
    printf(" kxx=%.8lf, kxy=%.8lf, kyx=%.8lf, kyy=%.8lf\n",rxx,rxy,ryx,ryy);


    bool extreme=true;
    if(rxx<0.03 && rxx>0.002 && ryy<0.03 && ryy>0.002)
        extreme=false;

    if(extreme){
        adjust.posBitmap=0;
        for(int i=0;i<4;i++){
            qtObjects.lb_adjPosX[i]->clear();
            qtObjects.lb_adjPosY[i]->clear();
        }
        QMessageBox::warning(this,QString::fromUtf8("警告"),
                                    QString::fromUtf8("校正方法有误,导致\n数值异常，校正失败"),
                                    QMessageBox::Yes);
        return;
    }

    kxx=param.kxx;
    kxy=param.kxy;
    kyx=param.kyx;
    kyy=param.kyy;
    param.kxx=qxx*kxx+qxy*kyx;
    param.kxy=qxx*kxy+qxy*kyy;
    param.kyx=qyx*kxx+qyy*kyx;
    param.kyy=qyx*kxy+qyy*kyy;
    sprintf(buf,"%.8lf",param.kxx);
    write_profile_string( "MARK", "MM_PER_PIXEL_WW",buf, EMC_INIFILE);
    sprintf(buf,"%.8lf",param.kxy);
    write_profile_string( "MARK", "MM_PER_PIXEL_WH",buf, EMC_INIFILE);
    sprintf(buf,"%.8lf",param.kyx);
    write_profile_string( "MARK", "MM_PER_PIXEL_HW",buf, EMC_INIFILE);
    sprintf(buf,"%.8lf",param.kyy);
    write_profile_string( "MARK", "MM_PER_PIXEL_HH",buf, EMC_INIFILE);
    *transfMatrix=ImageActualTM( TransformMatrix(param.kxx, param.kxy, param.kyx, param.kyy),
                                 srcImage->width, srcImage->height, 0, 0);
    printf("CCD0 param: kxx=%.8lf, kxy=%.8lf, kyx=%.8lf, kyy=%.8lf\n", param.kxx,param.kxy,param.kyx,param.kyy);

    adjust.posBitmap=0;
    for(int i=0;i<4;i++){
        qtObjects.lb_adjPosX[i]->clear();
        qtObjects.lb_adjPosY[i]->clear();
    }
}

void MarkWidget::focus_point_select(int x, int y){
    if(tabWidget->currentIndex()==2){
        focusAidView->set_left_top(cvPoint(x, y));
    }
}

void MarkWidget::set_focus_times(bool checked){
    if(checked){
        if(bt_4x==qobject_cast<QPushButton*>(sender())){
            focusAidView->set_zoom_times(4);
            bt_8x->setChecked(false);
        }
        else  if(bt_8x==qobject_cast<QPushButton*>(sender())){
            focusAidView->set_zoom_times(8);
            bt_4x->setChecked(false);
        }

    }
}

void MarkWidget::set_camera_param(int val){  
    if(sp_ADLevle==qobject_cast<QSpinBox *>( sender())){
        param.camADL=val;
        if(capture)
            capture->set_AD_level(val);
        write_profile_int("MARK","CAM_ADC_LEVEL",val, EMC_INIFILE);
    }
    else{
        QSlider* slider=qobject_cast<QSlider *>( sender());
        if(slider==sl_contrast){
            param.camGain=val;
            if(capture)
                capture->set_contrast(val);
            write_profile_int("MARK","CAM_GAIN",val, EMC_INIFILE);
        }
        else if(slider==sl_brightness){
            if(cb_autoBL->isChecked())
                return;
            param.camBL=val;
            if(capture)
                capture->set_brightness(val);
            write_profile_int("MARK","CAM_BLACK_LEVEL",val, EMC_INIFILE);
        }
        else if(slider==sl_exposure){
            param.camExposure=val;
            if(capture)
                capture->set_exposure_time(val);
            write_profile_int("MARK","CAM_EXPOSURE",val, EMC_INIFILE);
        }
    }
    //printf("val=%d\n", val);
}

void MarkWidget::load_image()//for debug
{
    QString fileName = QFileDialog::getOpenFileName(this, QString::fromUtf8("载入图片"),
                                                     "/home/u",
                                                     tr("Images (*.ppm *.png *.bmp *.jpg)"));
    if(fileName.size()>0){
        IplImage* openImage=cvLoadImage(fileName.toUtf8().constData(),0);
        cvResize(openImage, srcImage);
        cvReleaseImage(&openImage);
    }
}


void MarkWidget::save_image()
{
    QString fileName = QFileDialog::getSaveFileName(this, QString::fromUtf8("保存图片"),
                                "/home/u/untitled.ppm",
                                tr("Images (*.ppm *.png *.bmp *.jpg)"));
    if(fileName.size()>0)
        cvSaveImage(fileName.toUtf8().constData(), srcImage);
}

void MarkWidget::auto_brightness(int val)
{
    bool checked = cb_autoBL->isChecked();
    if(checked)
    {
        capture->set_auto_BL(1);
    }
    else
    {
        capture->set_auto_BL(0);
    }
}

void MarkWidget::machine_open(int cmd){
#ifdef WITH_EMC
    if(cmd){
        emc_estop(NULL,0);
        emc_machine(NULL,1);
        emc_mode(NULL,EMC_TASK_MODE_MANUAL);
        emc_optional_stop(NULL,1);
    }
    else{
        emc_machine(NULL,0);
    }
#endif
}

void MarkWidget::home(){
#ifdef WITH_EMC
    emc_unhome(0);
    emc_unhome(1);
    emc_unhome(2);
    emc_unhome(3);
    emc_unhome(4);
    emcStatus.homing = true;
#endif
}

void MarkWidget::zero(){
#ifdef WITH_EMC
    char buf[64];
    emc_mode(NULL,EMC_TASK_MODE_MDI);
    sprintf(buf,"g0 g53 z0");
    emc_mdi(buf);
    sprintf(buf,"g0 g53 x0 y0 a0 b0");
    emc_mdi(buf);
    emcStatus.stopToManual=true;
#endif
}

void MarkWidget::set_fast_velocity(double vel){
    param.fastVel = vel;
    write_profile_double("DISPLAY", "MAX_LINEAR_VELOCITY", param.fastVel,EMC_INIFILE);
}

void MarkWidget::set_slow_velocity(double vel){
    param.slowVel = vel;
    write_profile_double("DISPLAY", "MIN_LINEAR_VELOCITY", param.slowVel,EMC_INIFILE);
}

void MarkWidget::jog(int axis, double velocity){
#ifdef WITH_EMC
    emc_jog(axis,velocity);
#endif
}

void MarkWidget::end_jog(int axis){
#ifdef WITH_EMC
    emc_jog_stop(axis);
#endif
}

void MarkWidget::scan_diamon() {
    scan_test();
}

void MarkWidget::scan_watch() {
    cam_run();
}

void MarkWidget::set_glue() {
    set_all_holes();
}

void MarkWidget::set_diamond()
{
    set_diamond_in_all_holes();
}

void MarkWidget::auto_run(bool type)
{
    if(type)
        auto_set_diamond();
    else
        half_auto_set_diamond();
}

void MarkWidget::pause()
{
#ifdef WITH_EMC
    if(emcStatus.programStaut == EMC_TASK_INTERP_IDLE ||
            emcStatus.programStaut == EMC_TASK_INTERP_READING)
    emc_pause();
#endif
}

void MarkWidget::stop()
{
#ifdef WITH_EMC
    if(emcStatus.programStaut != EMC_TASK_INTERP_IDLE)
        emc_abort();
#endif
}

void MarkWidget::closeSystem()
{
    write_profile_string("MARK", "CURRENT_PROJECT", prjManage.get_current_project_name(),EMC_INIFILE);
}

void MarkWidget::set_var_param(int varNum, double value)
{
    switch(ProjectManage::VAR_PARAM(varNum)){
    case ProjectManage::PICKUP_Z:
        set_pickup_diamnod_z(value);
        break;
    case ProjectManage::SETDIAMOND_Z:
        set_setdiamond_z_pos(value);
        break;
    case ProjectManage::SET_GLUE_Z:
        set_glue_z_pos(value);
        break;
    case ProjectManage::GLUE_T:
        break;
    case ProjectManage::AFTER_GLUE_T:
        break;
    case ProjectManage::GET_DIAMOND_T:
        break;
    case ProjectManage::SET_DIAMOND_T:
        break;
    default:
        break;
    }
}

void MarkWidget::set_io(int index , bool on)
{
#ifdef WITH_EMC
    if(emcStatus.mode == EMC_TASK_MODE_MANUAL){
        char buf[64];
        if(on)
            sprintf(buf,"M64 P%d",index);
        else
            sprintf(buf,"M65 P%d",index);

        emc_mode(NULL,EMC_TASK_MODE_MDI);
        emc_mdi(buf);
        emc_mode(NULL,EMC_TASK_MODE_MANUAL);
    }

#endif
}

void MarkWidget::open_project()
{
    show_load_dialog();
}

void MarkWidget::new_project()
{
    if(newProjectDialog->exec()==QDialog::Accepted){
        //printf("New a project\nProject name: %s\n",newProjectDialog->get_project_name().toStdString().c_str());
        char dirBuf[512];
        char createDir[640];
        char copyFile[640];
        QString projectName = newProjectDialog->get_project_name();
        sprintf(dirBuf,"/home/u/cnc/镶钻存档/%s",projectName.toStdString().c_str());
        sprintf(createDir,"mkdir %s",dirBuf);
        sprintf(copyFile,"cp %s/* %s/",prjManage.project_dir(),dirBuf);
        int res = system(createDir);
        res += system(copyFile);
        if(res != 0){
            printf("Create new project failed!\n");
            return;
        }
        load_project(projectName.toStdString().c_str());
    }
}


