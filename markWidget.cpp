
#include <QtGui>
#include "markWidget.h"
#include "emcsec.hh"
#include "iniwrite.hh"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>

#define MARK2_VERSION "WATCH 1.0.0"

#define OFFSET_WHITE_CIRCLE 0
#define OFFSET_BLACK_CIRCLE 1
#define OFFSET_CENTER	2
#define CCDS_RELATE 3

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
#else
    param.load("/home/u/cnc/configs/ppmc/ppmc.ini");
#endif
    prjManage.load(param.projectName);
    setupUi(this);
    get_qt_objects();
    start_Capture(param.camGain, param.camBL,param.camExposure,param.camADL, 2);
    detecter_model_init();
    offsetSetting= new OffsetSetting(param.camRelx, param.camRely, param.glueRelx, param.glueRely);

    markView= new MarkView(widgetMarkView);
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
    halTimer->start(20);
#endif
}

MarkWidget::~MarkWidget(){
    emc_quit();
    hal_exit(halData->comp_id);
    printf("mark exit\n");
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
        srcImage->width, srcImage->height, param.camRelx, param.camRely);
    circlesDetecter = new CirclesDetecter;
    circlesDetecter->set_pattern(prjManage.get_diamond_pattern());
    circlesDetecter->set_area(&prjManage.searcRectD);
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
    loadRecordDialog = new LoadRecordDialog(this);
    char buf[16];
    sprintf(buf, "%8.3f", prjManage.scanStartPos[0]);
    lb_scanX0->setText(QString::fromUtf8(buf));
    sprintf(buf, "%8.3f", prjManage.scanStartPos[1]);
    lb_scanY0->setText(QString::fromUtf8(buf));
    sprintf(buf, "%8.3f", prjManage.scanStartPos[2]);
    lb_scanZ0->setText(QString::fromUtf8(buf));

    doubleValidator = new QDoubleValidator(this);
    intValidator = new QIntValidator(this);
    intValidator->setBottom(1);

    le_rowDis->setValidator(doubleValidator);
    sprintf(buf, "%.3f", prjManage.scanRowDis);
    le_rowDis->setText(buf);
    le_colDis->setValidator(doubleValidator);
    sprintf(buf, "%.3f", prjManage.scanColDis);
    le_colDis->setText(buf);
    le_scanRows->setValidator(intValidator);
    le_scanRows->setText(QString::number(prjManage.scanRowNum));
    le_scanCols->setValidator(intValidator);
    le_scanCols->setText(QString::number(prjManage.scanColNum));
    le_pickupZD->setValidator(doubleValidator);
    sprintf(buf,"%.3f", prjManage.pickupZD);
    le_pickupZD->setText(QString::fromUtf8(buf));
    sprintf(buf,"%.3f", prjManage.sendZD);
    le_sendZD->setText(QString::fromUtf8(buf));

    connect(tb_selectPattern0, SIGNAL(toggled(bool)), this, SLOT(select_pattern_toggled(bool)));
    connect(tb_searchArea0, SIGNAL(toggled(bool)), this, SLOT(search_area_toggled(bool)));
    connect(bt_detect0,SIGNAL(toggled(bool)), this, SLOT(diamond_test_toggled(bool)));
    connect(bt_setScanStart, SIGNAL(clicked()), this, SLOT(set_scan_beginning()));
    connect(bt_gotoScanStart, SIGNAL(clicked()), this, SLOT(back_scan_beginning()));
    connect(bt_scanTest,SIGNAL(clicked()), this, SLOT(scan_test()));
    connect(bt_setPickupZD, SIGNAL(clicked()), this, SLOT(set_pickup_diamnod_z()));
    connect(bt_setSendZD, SIGNAL(clicked()), this, SLOT(set_send_diamnod_z()));
    connect(bt_pickupFirst, SIGNAL(clicked()), this, SLOT(pickup_first()));
    connect(bt_pickupNext, SIGNAL(clicked()), this, SLOT(pickup_next()));
    connect(bt_pickupAll, SIGNAL(clicked()), this, SLOT(pickup_all()));
}

void MarkWidget::watch_page_init()
{
    char buf[16];
    le_rotateDeg->setValidator(doubleValidator);
    sprintf(buf, "%.3f",param.degInc);
    le_rotateDeg->setText(QString::fromUtf8(buf));
    connect(bt_rotateForward, SIGNAL(clicked()), this, SLOT(change_angle()));
    connect(bt_rotateBackward, SIGNAL(clicked()), this, SLOT(change_angle()));
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
}


void MarkWidget::adjust_page_init(){
     char buf[16];
     sprintf(buf, "%8.3f", param.camRelx);
     lb_ccdOffsetX->setText(buf);
     sprintf(buf, "%8.3f", param.camRely);
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
     connect(bt_setOffset,SIGNAL(clicked()), this, SLOT(set_ccd_offset_pressed()));
}


void MarkWidget::mark_view_update(){

    static int autoIndex=-1;
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
    }
    else{
        if(autoIndex!=watchResult.scanIndex){
            autoIndex=watchResult.scanIndex;
            markView->receive_image(srcImage);
            markView->set_diamond_pos(circlesDetecter->get_positions());
            markView->set_search_frame(prjManage.searcRectD);
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

    if(circlesDetecter->pattern_is_new()){
        const IplImage* pattern=circlesDetecter->get_pattern();
        if(pattern){
            patternView->receive_image(pattern);
            patternView->update();
        }
    }
}


void MarkWidget::slow_cycle(){

#ifdef WITH_EMC
    emcStatus.update();
    if(emcStatus.stopToManual && emcStatus.hasStop
            && emcStatus.programStaut==EMC_TASK_INTERP_IDLE){
        emc_mode(NULL, EMC_TASK_MODE_MANUAL);
        emcStatus.stopToManual=false;
    }
#endif

    if(bt_detect0->isChecked()){
        circlesDetecter->detect(srcImage);
        markView->set_diamond_pos(circlesDetecter->get_positions());
    }

    cv_cmd_cycle();
}

void MarkWidget::auto_detect_diamond(){
    if(capture)
        capture->get_image(srcImage);
    circlesDetecter->detect(srcImage);
    const list<Point>& imgPos =circlesDetecter->get_positions();
    watchResult.scanIndex++;

    list<Point>::const_iterator it;
    Point pos;
    Vector2 vm(emcStatus.cmdAxis[0], emcStatus.cmdAxis[1]);
    for(it=imgPos.begin();it!=imgPos.end();it++){
        pos=transfMatrix->transform(it->x(), it->y());
        pos.move(vm);
        watchResult.dimamondPos.push_back(pos);
    }
    markView->set_diamond_sum((int)watchResult.dimamondPos.size());
}


void MarkWidget::clear_diamond_pos(){
    watchResult.dimamondPos.clear();
    watchResult.scanIndex=0;
    markView->set_diamond_pos(watchResult.dimamondPos);
    markView->set_diamond_sum(0);
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
          auto_detect_diamond();
        }
        *halpins->cvCmd=0;
        state=MARK_IDLE;
        break;

    default:
        state=MARK_IDLE;
    }
}

void MarkWidget::fast_react_cycle(){
     MarkHalPins* halpins=halData->halpins;

    if(*halpins->posCmd==1){
        Point pos= *watchResult.dimamondPos.begin();
        *halpins->posAxis[0]=pos.x();
        *halpins->posAxis[1]=pos.y();
        *halpins->posAxis[2]=prjManage.pickupZD;
        *halpins->posCmd=0;
    }

    if(*halpins->reachCmd==1){
        watchResult.dimamondPos.pop_front();
        *halpins->reachCmd=0;
        markView->set_diamond_sum((int)watchResult.dimamondPos.size());
    }

    if(watchResult.dimamondPos.size()>0)
        *halpins->posValid=1;
    else
        *halpins->posValid=0;
}

void MarkWidget::ready_for_diamond_scan(){
    clear_diamond_pos();
    sp_ADLevle->setValue(prjManage.adlD);
    sl_contrast->setValue(prjManage.contrastD);
    sl_brightness->setValue(prjManage.brightnessD);
    sl_exposure->setValue(prjManage.exposureD);
}

void MarkWidget::select_pattern_toggled(bool checked){
    RectangleFrame frame;
    QPushButton* button=qobject_cast<QPushButton*>(sender());
    if(button==tb_selectPattern0){
        if(checked)
            markView->set_default_pattern_frame();
        else{
            frame=markView->get_pattern_frame();
            Point lt=frame.get_top_left();
            int pw=frame.get_width()+0.5;
            int ph=frame.get_height()+0.5;
            CvRect rect=cvRect(lt.x()+0.5, lt.y()+0.5, pw, ph);
            circlesDetecter->set_pattern(srcImage, &rect);
            prjManage.save_diamond_pattern(circlesDetecter->get_pattern());
        }
    }

}

void MarkWidget::search_area_toggled(bool checked){
    QPushButton* button=qobject_cast<QPushButton*>(sender());
    if(button==tb_searchArea0){
        if(checked)
            markView->set_default_search_frame();
        else{
            RectangleFrame frame= markView->get_search_frame();
            Point lt=frame.get_top_left();
            int pw=frame.get_width()+0.5;
            int ph=frame.get_height()+0.5;
            prjManage.searcRectD=cvRect(lt.x()+0.5, lt.y()+0.5, pw, ph);
            circlesDetecter->set_area(&prjManage.searcRectD);
            prjManage.save_diamond_search_area();
        }
    }
}

void MarkWidget::diamond_test_toggled(bool checked){
    if(checked){
        markView->set_search_frame(prjManage.searcRectD);
        prjManage.save_diamond_camera_param(param.camADL, param.camBL, param.camGain, param.camExposure);
    }
    else{
        list<Point> empty;
        markView->set_diamond_pos(empty);
    }
}


void MarkWidget::set_scan_beginning(){
    char buf[32];
    double pos[3];
    int i;
    for(i=0;i<3;i++)
        pos[i]=emcStatus.cmdAxis[i];
#ifdef WITH_EMC
    emc_mode(NULL, EMC_TASK_MODE_MDI);
    for(i=0;i<3;i++){
        if(fabs(prjManage.scanStartPos[i]-pos[i])>0.0005){
            prjManage.scanStartPos[i]=pos[i];
            sprintf(buf, "#%d=%f", ProjectManage::SCAN_X0+i, pos[i]);
            emc_mdi(buf);
        }
    }
    emc_mode(NULL, EMC_TASK_MODE_MANUAL);
#endif
    sprintf(buf, "%8.3f", pos[0]);
    lb_scanX0->setText(QString::fromUtf8(buf));
    sprintf(buf, "%8.3f", pos[1]);
    lb_scanY0->setText(QString::fromUtf8(buf));
    sprintf(buf, "%8.3f", pos[2]);
    lb_scanZ0->setText(QString::fromUtf8(buf));
}

void MarkWidget::back_scan_beginning(){
    char buf[64];
#ifdef WITH_EMC
    emc_mode(NULL, EMC_TASK_MODE_MDI);
    emc_mdi("g0 g53 z0");
    sprintf(buf, "g0 g53 x%f y%f", prjManage.scanStartPos[0], prjManage.scanStartPos[1]);
    emc_mdi(buf);
    sprintf(buf, "g1 g53 z%f f3000", prjManage.scanStartPos[2]);
    emc_mdi(buf);
    emcStatus.stopToManual=true;
#endif
}


void MarkWidget::scan_test(){
    int rn, cn;
    double rd, cd;
    char buf[32];

    rn=le_scanRows->text().toInt();
    cn=le_scanCols->text().toInt();
    rd=le_rowDis->text().toDouble();
    cd=le_colDis->text().toDouble();
#ifdef WITH_EMC
    emc_mode(NULL, EMC_TASK_MODE_MDI);

    if(rn!=prjManage.scanRowNum){
        sprintf(buf, "#%d=%d", ProjectManage::SCAN_ROW_NUM, rn);
        emc_mdi(buf);
    }
    if(cn!=prjManage.scanColNum){
        sprintf(buf, "#%d=%d", ProjectManage::SCAN_COL_NUM, cn);
        emc_mdi(buf);
    }
    if(fabs(rd-prjManage.scanRowDis)>0.0005){
        sprintf(buf, "#%d=%f", ProjectManage::SCAN_ROW_DIS, rd);
        emc_mdi(buf);
    }
    if(fabs(cd-prjManage.scanColDis)>0.0005){
        sprintf(buf, "#%d=%f", ProjectManage::SCAN_COL_DIS, cd);
        emc_mdi(buf);
    }
    emc_mode(NULL, EMC_TASK_MODE_AUTO);
    emc_open("/home/u/cnc/configs/ppmc/o_nc/scan.ngc");
    emc_wait("done");
    emc_run(0);
    emcStatus.stopToManual=true;
#endif
    prjManage.scanRowNum=rn;
    prjManage.scanColNum=cn;
    prjManage.scanRowDis=rd;
    prjManage.scanColDis=cd;
    prjManage.save_scan_param();
}

void MarkWidget::set_pickup_diamnod_z(){
    char buf[32];
    prjManage.pickupZD=emcStatus.cmdAxis[2];
    sprintf(buf, "%.3f", prjManage.pickupZD);
    le_pickupZD->setText(QString::fromUtf8(buf));
    write_profile_double("DIAMOND", "PICKUP_Z", prjManage.pickupZD, prjManage.ini_file());
#ifdef WITH_EMC
    emc_mode(NULL,EMC_TASK_MODE_MDI);
    sprintf(buf, "#%d=%f", ProjectManage::PICKUP_Z, prjManage.pickupZD);
    emc_mdi(buf);
    emc_mode(NULL,EMC_TASK_MODE_MANUAL);
#endif
}

void MarkWidget::set_send_diamnod_z(){
    char buf[32];
    prjManage.sendZD=emcStatus.cmdAxis[2];
    sprintf(buf, "%.3f", prjManage.sendZD);
    le_sendZD->setText(QString::fromUtf8(buf));
    write_profile_double("DIAMOND", "SEND_Z", prjManage.sendZD, prjManage.ini_file());
#ifdef WITH_EMC
    emc_mode(NULL,EMC_TASK_MODE_MDI);
    sprintf(buf, "#%d=%f", ProjectManage::SEND_ZD, prjManage.sendZD);
    emc_mdi(buf);
    emc_mode(NULL,EMC_TASK_MODE_MANUAL);
#endif
}


void MarkWidget::pickup_first(){
    char buf[128];
    if(watchResult.dimamondPos.size()==0)
        return;
    watchResult.pickupIterator=watchResult.dimamondPos.begin();
    Point pos=*watchResult.pickupIterator;
#ifdef WITH_EMC
    emc_mode(NULL,EMC_TASK_MODE_MDI);
    emc_mdi("g0 g53 z0");
    sprintf(buf, "g0 g53 x%f y%f", pos.x(), pos.y());
    emc_mdi(buf);
    sprintf(buf, "g1 g53 z%f f3000", prjManage.pickupZD);
    emc_mdi(buf);
    emcStatus.stopToManual=true;
#endif
}

void MarkWidget::pickup_next(){
    char buf[128];
    if(watchResult.dimamondPos.size()==0)
        return;
    watchResult.pickupIterator++;
    if(watchResult.pickupIterator==watchResult.dimamondPos.end()){
       watchResult.pickupIterator=watchResult.dimamondPos.begin();
    }
    Point pos= *watchResult.pickupIterator;
#ifdef WITH_EMC
    emc_mode(NULL,EMC_TASK_MODE_MDI);
    sprintf(buf, "g0 g53 z%f", prjManage.sendZD);
    emc_mdi(buf);
    sprintf(buf, "g0 g53 x%f y%f", pos.x(), pos.y());
    emc_mdi(buf);
    sprintf(buf, "g1 g53 z%f f3000", prjManage.pickupZD);
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




void MarkWidget::show_load_dialog(){
    loadRecordDialog->set_combo_box(param.projectName);
    if(loadRecordDialog->exec()==QDialog::Accepted){
       QString prjName= loadRecordDialog->get_project_name();
       if(prjName.size()>0){
           //load_project(prjName.toUtf8().constData());
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
        res=circlesDetecter->detect(srcImage);
        if(!res){
            pos=*circlesDetecter->get_positions().begin();
            markView->set_diamond_pos(list<Point>(1, pos));
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
    offsetSetting->holePosOk=true;
    offsetSetting->holeDetectOk=0;
    offsetSetting->holePos[0]=x;
    offsetSetting->holePos[1]=y;
    sprintf(buf, "%8.3lf", x);
    lb_holeX->setText(buf);
    sprintf(buf, "%8.3lf", y);
    lb_holeY->setText(buf);

}

void MarkWidget::detect_hole_presssed(){
    int res=0;
    double cx=0,cy=0;
    char buf[32];
    Point pos;
    Vector2 vm(emcStatus.cmdAxis[0], emcStatus.cmdAxis[1]);
    if(rb_circle->isChecked()){
        res=circlesDetecter->detect(srcImage);
        if(!res){
            pos=*circlesDetecter->get_positions().begin();
            markView->set_diamond_pos(list<Point>(1, pos));
            pos=transfMatrix->transform(pos.x(), pos.y());
            pos.move(vm);
            cx=pos.x();
            cy=pos.y();
        }
    }


    if(!res){
        offsetSetting->holeDetePos[0]=cx;
        offsetSetting->holeDetePos[1]=cy;
        offsetSetting->holeDetectOk=true;
        sprintf(buf, "%8.3lf", cx);
        lb_holeDetectX->setText(buf);
        sprintf(buf, "%8.3lf", cy);
        lb_holeDetectY->setText(buf);

        if(offsetSetting->holePosOk){
            offsetSetting->ccdOffset[0]=offsetSetting->holePos[0]-offsetSetting->holeDetePos[0]+param.camRelx;
            offsetSetting->ccdOffset[1]=offsetSetting->holePos[1]-offsetSetting->holeDetePos[1]+param.camRely;
            sprintf(buf, "%8.3lf", offsetSetting->ccdOffset[0]);
            lb_ccdOffsetX->setText(buf);
            sprintf(buf, "%8.3lf", offsetSetting->ccdOffset[1]);
            lb_ccdOffsetY->setText(buf);
        }
    }
    else{
        offsetSetting->holeDetectOk=false;
        lb_holeDetectX->setText(QString::fromUtf8("无效"));
        lb_holeDetectY->setText(QString::fromUtf8("无效"));
        lb_ccdOffsetX->clear();
        lb_ccdOffsetY->clear();
    }
}

void MarkWidget::input_glue_pos_pressed(){
    char buf[32];
    offsetSetting->gluePos[0]=emcStatus.cmdAxis[0];
    offsetSetting->gluePos[1]=emcStatus.cmdAxis[1];
    sprintf(buf, "%8.3lf", offsetSetting->gluePos[0]);
    lb_glueX->setText(buf);
    sprintf(buf, "%8.3lf", offsetSetting->gluePos[1]);
    lb_glueY->setText(buf);

    offsetSetting->glueOffset[0]=offsetSetting->holePos[0]-offsetSetting->gluePos[0];
    offsetSetting->glueOffset[1]=offsetSetting->holePos[1]-offsetSetting->gluePos[1];
    sprintf(buf, "%8.3lf", offsetSetting->glueOffset[0]);
    lb_glueOffsetX->setText(buf);
    sprintf(buf, "%8.3lf", offsetSetting->glueOffset[1]);
    lb_glueOffsetY->setText(buf);
}

void MarkWidget::set_ccd_offset_pressed()
{

    int r=QMessageBox::information(this,QString::fromUtf8("提示"),
                                   QString::fromUtf8("修改并保存偏移值"),
                                   QMessageBox::Yes|QMessageBox::No);
    if(r==QMessageBox::Yes){
        param.camRelx=offsetSetting->ccdOffset[0];
        param.camRely=offsetSetting->ccdOffset[1];
        write_profile_double( "MARK", "CAM_REL_SPINDLE_X", param.camRelx, EMC_INIFILE);
        write_profile_double( "MARK", "CAM_REL_SPINDLE_Y", param.camRely, EMC_INIFILE);
        offsetSetting->holeDetectOk=0;
        *transfMatrix=ImageActualTM( TransformMatrix(param.kxx, param.kxy, param.kyx, param.kyy),
                                     srcImage->width, srcImage->height, param.camRelx, param.camRely);
        param.glueRelx=offsetSetting->glueOffset[0];
        param.glueRely=offsetSetting->glueOffset[1];
        write_profile_double( "MARK", "GLUE_REL_SPINDLE_X", param.glueRelx, EMC_INIFILE);
        write_profile_double( "MARK", "GLUE_REL_SPINDLE_Y", param.glueRely, EMC_INIFILE);
    }
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
                                 srcImage->width, srcImage->height, param.camRelx, param.camRely);
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