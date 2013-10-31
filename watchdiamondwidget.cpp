#include "watchdiamondwidget.h"
#include "ui_watchdiamondwidget.h"
#include "markEMC.h"
#include <QShortcut>
#include <stdio.h>
#include <stdlib.h>

WatchDiamondWidget::WatchDiamondWidget(int argc, char **argv, QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this);
    setFocusPolicy(Qt::StrongFocus);
    markWidget = new MarkWidget(argc,argv,wg_mark);
    coordinate = new Coordinate(lb_x,lb_y,lb_z,lb_a,lb_b);
    setupDialog= new AlgorithmDialog(this);

    for(int i=0; i<5; i++)
        axisPos[i] = 0;

    doubleValidator = new QDoubleValidator(this);
    le_setDiamondT->setValidator(doubleValidator);
    le_getDiamondT->setValidator(doubleValidator);
    le_glueT->setValidator(doubleValidator);
    le_afterGlueT->setValidator(doubleValidator);

    slowVel = markWidget->get_slow_velocity();
    fastVel = markWidget->get_fast_velocity();
    currentVel = fastVel;
    //QString vel = double_to_qstring(currentVel);
    //le_setVelocity->setText(vel);
    autoRun = false;

    QAction* clear = new QAction(QString::fromUtf8("清除所有"),this);
    te_sysMessage->addAction(clear);
    te_sysMessage->setContextMenuPolicy(Qt::ActionsContextMenu);
    te_sysMessage->setTextColor(QColor(Qt::red));
    te_sysMessage->setReadOnly(true);
    te_sysMessage->setText(QString("system Message:\n"));

    jogButtons[0] = bt_xPlus; jogButtons[1] = bt_xReduce; jogButtons[2] = bt_yPlus; jogButtons[3] = bt_yReduce;
    jogButtons[4] = bt_zPlus; jogButtons[5] = bt_zReduce; jogButtons[6] = bt_aPlus; jogButtons[7] = bt_aReduce;
    jogButtons[8] = bt_bPlus; jogButtons[9] = bt_bReduce;

    ioButtons[0] = tb_1; ioButtons[1] = tb_2; ioButtons[2] = tb_3;
    ioButtons[3] = tb_4; ioButtons[4] = tb_5; ioButtons[5] = tb_6;

    connect(markWidget,SIGNAL(update_emc_status(MarkEmcStatus)),this,SLOT(update_emc_slot(MarkEmcStatus)));    
    connect(markWidget,SIGNAL(update_infor(Information&)),this,SLOT(update_infor_slot(Information&)));
    connect(bt_machineRun,SIGNAL(toggled(bool)),this,SLOT(machine_open_toggled(bool)));
    connect(bt_home,SIGNAL(clicked()),this,SLOT(home()));
    connect(bt_zero,SIGNAL(clicked()),this,SLOT(zero()));
    connect(bt_changeVelocity,SIGNAL(toggled(bool)),this,SLOT(change_vel_toggled(bool)));
    //connect(le_setVelocity,SIGNAL(editingFinished()),this,SLOT(finish_set_velocity()));
    connect(bt_scanDiamond,SIGNAL(clicked()),this,SLOT(scan_diamond()));
    connect(bt_scanWatch,SIGNAL(toggled(bool)),this,SLOT(scan_watch(bool)));
    connect(bt_setGlue,SIGNAL(toggled(bool)),this,SLOT(set_glue(bool)));
    connect(bt_setDiamond,SIGNAL(toggled(bool)),this,SLOT(set_diamond(bool)));
    connect(action_open,SIGNAL(triggered()),this,SLOT(open_project()));
    connect(action_new,SIGNAL(triggered()),this,SLOT(new_project()));
    connect(action_halConfig,SIGNAL(triggered()),this,SLOT(hal_config()));
    connect(action_halMeter,SIGNAL(triggered()),this,SLOT(hal_meter()));
    connect(action_halScope,SIGNAL(triggered()),this,SLOT(hal_scope()));
    connect(action_algorithm,SIGNAL(triggered()),this,SLOT(setup_algorithm()));
    connect(bt_autoRun,SIGNAL(toggled(bool)),this,SLOT(auto_run(bool)));
    connect(bt_pause,SIGNAL(toggled(bool)),this,SLOT(pause(bool)));
    connect(bt_stop,SIGNAL(clicked()),this,SLOT(stop()));
    connect(clear,SIGNAL(triggered()),this,SLOT(clear_error_message()));

    connect(bt_getDiamondZ,SIGNAL(clicked()),this,SLOT(set_height()));
    connect(bt_setGlueZ,SIGNAL(clicked()),this,SLOT(set_height()));
    connect(bt_setDiamondZ,SIGNAL(clicked()),this,SLOT(set_height()));
    connect(sp_getDiamondZ,SIGNAL(editingFinished()),this,SLOT(set_height()));
    connect(sp_setDiamondZ,SIGNAL(editingFinished()),this,SLOT(set_height()));
    connect(sp_setGlueZ,SIGNAL(editingFinished()),this,SLOT(set_height()));

    connect(le_setDiamondT,SIGNAL(editingFinished()),this,SLOT(set_time()));
    connect(le_getDiamondT,SIGNAL(editingFinished()),this,SLOT(set_time()));
    connect(le_glueT,SIGNAL(editingFinished()),this,SLOT(set_time()));
    connect(le_afterGlueT,SIGNAL(editingFinished()),this,SLOT(set_time()));

    connect(hs_slowVel,SIGNAL(valueChanged(int)),this,SLOT(set_velocity(int)));
    connect(hs_fastVel,SIGNAL(valueChanged(int)),this,SLOT(set_velocity(int)));

    connect(sp_pickupOffsetX,SIGNAL(valueChanged(double)),this,SLOT(micro_adjust_offset(double)));
    connect(sp_pickupOffsetY,SIGNAL(valueChanged(double)),this,SLOT(micro_adjust_offset(double)));
    connect(sp_glueOffsetX,SIGNAL(valueChanged(double)),this,SLOT(micro_adjust_offset(double)));
    connect(sp_glueOffsetY,SIGNAL(valueChanged(double)),this,SLOT(micro_adjust_offset(double)));

    for(int i=0; i<10; i++){
        connect(jogButtons[i],SIGNAL(pressed()),this,SLOT(jog()));
        connect(jogButtons[i],SIGNAL(released()),this,SLOT(end_jog()));
    }

    for(int i=0; i<6; i++){
        connect(ioButtons[i],SIGNAL(toggled(bool)),this,SLOT(io_button_toggled(bool)));
    }

    connect(sp_distance,SIGNAL(valueChanged(double)),this,SLOT(set_diamond_distance(double)));
    sp_distance->setValue(0.080);
    bt_machineRun->setChecked(true);
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(ask_home()));
    timer->start(500);

    //bt_home->setChecked(true);
}

WatchDiamondWidget::~WatchDiamondWidget()
{
    delete markWidget;
}

void WatchDiamondWidget::update_emc_slot(const MarkEmcStatus& status)
{
    char buf[256];
    int colors[5];

    for(int i=0; i<5; i++){
        colors[i] = (int)status.axisHomeState[i];
        axisPos[i] = status.cmdAxis[i];
    }

    set_axis(status.cmdAxis,colors);
    if(status.errorMsg[0]!='\0'){
        sprintf(buf,"%s\n",status.errorMsg);
        te_sysMessage->append(QString::fromUtf8(buf));
    }
    if(status.operatorText[0]!='\0'){
        sprintf(buf,"%s\n",status.operatorText);
        te_sysMessage->append(QString::fromUtf8(buf));
    }
    if(status.operatorDisplay[0]!='\0'){
        sprintf(buf,"%s\n",status.operatorDisplay);
        te_sysMessage->append(QString::fromUtf8(buf));
    }
}

void WatchDiamondWidget::update_infor_slot(Information &infor){
    static int loadParam = 0;
    char buf[12];
    sprintf(buf,"%d",infor.diamondNum);
    lb_diamondNum->setText(QString(buf));
    sprintf(buf,"%d/%d",infor.watchPosIndex,infor.watchPosNum);
    lb_watchIndex->setText(QString(buf));
    sprintf(buf,"%d/%d",infor.gluePosIndex,infor.holePosNum);
    lb_glueIndex->setText(QString(buf));
    sprintf(buf,"%d/%d",infor.holePosIndex,infor.holePosNum);
    lb_holesIndex->setText(QString(buf));    
    lb_runTime->setText(infor.runTime);

    if(autoRun && infor.endAutoRun){
        autoRun = false;
        if(bt_autoRun->isChecked())
            bt_autoRun->setChecked(false);
    }
    if(infor.endSetGLue){
        if(bt_setGlue->isChecked())
            bt_setGlue->setChecked(false);
    }
    if(infor.endSetDiamond){
        if(bt_setDiamond->isChecked())
            bt_setDiamond->setChecked(false);
    }
    if(infor.endScanWatch){
        if(bt_scanWatch->isChecked())
            bt_scanWatch->setChecked(false);
    }

    for(int i=0; i<5; i++){
        if(infor.ioState[i])
            ioButtons[i]->setChecked(true);
        else
            ioButtons[i]->setChecked(false);
    }

    if(infor.start){
        if(!bt_autoRun->isChecked())
            bt_autoRun->setChecked(true);
    }
    if(infor.paused){
        if(!bt_pause->isChecked())
            bt_pause->setChecked(true);
        if(bt_autoRun->isChecked())
            bt_autoRun->setChecked(false);
    }
    if(infor.stop){
        bt_stop->click();
        //std::cout<<"test for one time"<<std::endl;
    }

    //update var parameters
    if(infor.changeProject){
        double values[7] = {infor.getDiamondZ,infor.setDiamondZ,infor.setGlueZ,
                           infor.glueT,infor.afterGlueT,infor.getDiamondT,infor.setDiamondT};
        set_var_params(values);
    }

    //只执行一次
    if(loadParam == 0 || infor.changeProject){
        sp_setGlueZ->setValue(infor.setGlueZ);
        sp_setDiamondZ->setValue(infor.setDiamondZ);
        sp_getDiamondZ->setValue(infor.getDiamondZ);
        sprintf(buf,"%.3f",infor.glueT);
        le_glueT->setText(buf);
        sprintf(buf,"%.3f",infor.afterGlueT);
        le_afterGlueT->setText(buf);
        sprintf(buf,"%.3f",infor.setDiamondT);
        le_setDiamondT->setText(buf);
        sprintf(buf,"%.3f",infor.getDiamondT);
        le_getDiamondT->setText(buf);
        sprintf(buf,"%.1f",infor.slowVel);
        lb_slowVel->setText(buf);
        sprintf(buf,"%d",(int)infor.fastVel);
        lb_faseVel->setText(buf);
        hs_slowVel->setValue((int)(infor.slowVel*10));
        hs_fastVel->setValue((int)infor.fastVel);
        sp_pickupOffsetX->setValue(infor.pickupOffsetX);
        sp_pickupOffsetY->setValue(infor.pickupOffsetY);
        sp_glueOffsetX->setValue(infor.glueOffsetX);
        sp_glueOffsetY->setValue(infor.glueOffsetY);
        loadParam++;
        infor.changeProject = false;
    }

}

void WatchDiamondWidget::set_axis(const double* val, int* colors)
{
    for(int i=0 ; i<5; i++)
        coordinate->set_value(i,*(val+i), *(colors+i));
}

void WatchDiamondWidget::machine_open_toggled(bool checked){
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if(button == bt_machineRun){
        if(checked){
            markWidget->machine_open(1);
            button->setText(QString::fromUtf8("机器开"));
        }
        else{
            markWidget->machine_open(0);
            button->setText(QString::fromUtf8("机器关"));
        }
    }

}

void WatchDiamondWidget::home()
{
    int r=QMessageBox::information(this,QString::fromUtf8("回零对话框"),
                                QString::fromUtf8("所有轴回零？"),
                                QMessageBox::Yes|QMessageBox::No);
    if(QMessageBox::Yes==r){
        markWidget->home();
    }
}

void WatchDiamondWidget::ask_home()
{
    bt_home->click();
    timer->stop();
}

void WatchDiamondWidget::zero()
{
    markWidget->zero();
}

void WatchDiamondWidget::change_vel_toggled(bool checked)
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if(button == bt_changeVelocity){
        if(checked){
            button->setText(QString::fromUtf8("慢速"));
            currentVel = slowVel;
        }
        else{
            button->setText(QString::fromUtf8("快速"));
            currentVel = fastVel;            
        }
    }

}


void WatchDiamondWidget::set_velocity(int value)
{
    char buf[32];
    QSlider* sd = qobject_cast<QSlider*>(sender());
    if(sd == hs_slowVel){
        //int value = hs_slowVel->value();
        slowVel = (double)value/10;
        sprintf(buf,"%.1f",slowVel);
        lb_slowVel->setText(buf);
        markWidget->set_slow_velocity(slowVel);

    }
    else if(sd == hs_fastVel){
        //int value = hs_fastVel->value();
        fastVel = value;
        sprintf(buf,"%d",value);
        lb_faseVel->setText(buf);
        markWidget->set_fast_velocity(fastVel);
    }
}

void WatchDiamondWidget::jog()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    int axis=0, i=0;
    double velocity = currentVel;//increase

    for(; i<10; i++){
        if(button == jogButtons[i]){
            axis = i;
            if(i%2 == 1){
                velocity *= (-1);
                axis = (i-1)/2;
            }
            else
                axis = i/2;

            break;
        }
    }

    markWidget->jog(axis,velocity);
}

void WatchDiamondWidget::end_jog()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    int axis=0, i=0;

    for(;i<10;i++){
        if(button == jogButtons[i]){
            if(i%2 == 1){
                axis = (i-1)/2;
            }
            else
                axis = i/2;
            break;
        }
    }

    markWidget->end_jog(axis);
}

void WatchDiamondWidget::io_button_toggled(bool checked)
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    for(int i=0; i<6; i++){
        if(button == ioButtons[i])
            markWidget->set_io(i,checked);
    }
}

void WatchDiamondWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->isAutoRepeat())
        return;

    int axis = 0;
    bool jog = true;
    double velocity = currentVel;
    switch(event->key())
    {
    case Qt::Key_Left:
        axis = 0;
        velocity *= (-1);
        break;
    case Qt::Key_Right:
        axis = 0;
        break;

    case Qt::Key_Up:
        axis = 1;        
        break;
    case Qt::Key_Down:
        axis = 1;
        velocity *= (-1);
        break;

    case Qt::Key_PageUp:
        axis = 2;
        break;
    case Qt::Key_PageDown:
        axis = 2;
        velocity *= (-1);
        break;

    case Qt::Key_W:
        axis = 3;
        break;
    case Qt::Key_S:
        axis = 3;
        velocity *= (-1);
        break;

    case Qt::Key_A:
        axis = 4;
        break;
    case Qt::Key_D:
        axis = 4;
        velocity *= (-1);
        break;
    case Qt::Key_Space:
        jog = false;
        if(bt_changeVelocity->isChecked())
            bt_changeVelocity->setChecked(false);
        else
            bt_changeVelocity->setChecked(true);
        break;
    default:
        jog = false;
        break;
    }
    if(jog)
        markWidget->jog(axis,velocity);
}

void WatchDiamondWidget::keyReleaseEvent(QKeyEvent *event)
{
    if(event->isAutoRepeat())
        return;
    int axis = 0;
    bool jog = true;
    switch(event->key())
    {
    case Qt::Key_Left:
    case Qt::Key_Right:
        axis = 0;
        break;

    case Qt::Key_Up:
    case Qt::Key_Down:
        axis = 1;
        break;

    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
        axis = 2;
        break;

    case Qt::Key_W:
    case Qt::Key_S:
        axis = 3;
        break;

    case Qt::Key_A:
    case Qt::Key_D:
        axis = 4;
        break;
    default:
        jog = false;
        break;
    }
    if(jog)
        markWidget->end_jog(axis);

}

void WatchDiamondWidget::closeEvent(QCloseEvent *event){
    int r = QMessageBox::information(this,QString::fromUtf8("关闭对话框"),
                                     QString::fromUtf8("关闭系统？"),
                                     QMessageBox::Yes|QMessageBox::No);
    if(QMessageBox::Yes==r){
        //markWidget->closeSystem();
        close();
    }
    else{
        event->ignore();
    }
}

void WatchDiamondWidget::scan_diamond()
{
    markWidget->scan_diamon();
}

void WatchDiamondWidget::scan_watch(bool checked)
{
    if(checked){
        markWidget->scan_watch();
        bt_pause->setChecked(false);
    }
}

void WatchDiamondWidget::set_glue(bool checked)
{
    if(checked){
        markWidget->set_glue();
        bt_pause->setChecked(false);
    }
}

void WatchDiamondWidget::set_diamond(bool checked)
{
    if(checked){
        markWidget->set_diamond();
        bt_pause->setChecked(false);
    }
}

void WatchDiamondWidget::auto_run(bool checked)
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if(button == bt_autoRun){
        if(checked){
            bool type = cb_scanWatch->isChecked();
            markWidget->auto_run(type);
            bt_pause->setChecked(false);
            autoRun = true;
        }
        else{
            autoRun = false;
        }
    }
}

void WatchDiamondWidget::pause(bool checked)
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if(button == bt_pause){
        if(checked){
            markWidget->pause();
            bt_autoRun->setChecked(false);
            bt_setGlue->setChecked(false);
            bt_setDiamond->setChecked(false);
            bt_scanWatch->setChecked(false);

        }
    }
}

void WatchDiamondWidget::stop()
{
    markWidget->stop();
    bt_autoRun->setChecked(false);
    bt_pause->setChecked(false);
}

void WatchDiamondWidget::set_height()
{
    QPushButton* bt = qobject_cast<QPushButton*>(sender());
    QDoubleSpinBox* sp = qobject_cast<QDoubleSpinBox*>(sender());
    if(bt == bt_getDiamondZ){
        sp_getDiamondZ->setValue(axisPos[2]);
        set_var_param(3308,0);
    }
    else if(bt == bt_setDiamondZ){
        sp_setDiamondZ->setValue(axisPos[2]);
        set_var_param(3309,0);
    }
    else if(bt == bt_setGlueZ){
        sp_setGlueZ->setValue(axisPos[2]);
        set_var_param(3310,0);
    }
    else if(sp == sp_getDiamondZ){
        set_var_param(3308,sp->value());
    }
    else if(sp == sp_setDiamondZ){
        set_var_param(3309,sp->value());
    }
    else if(sp == sp_setGlueZ){
        set_var_param(3310,sp->value());
    }
}

void WatchDiamondWidget::set_time()
{
    QLineEdit* le = qobject_cast<QLineEdit*>(sender());
    double value = le->text().toDouble();

    if(le == le_glueT){
        markWidget->set_time(0,3311,value);
    }
    else if(le == le_afterGlueT){
        markWidget->set_time(1,3312,value);

    }
    else if(le == le_getDiamondT){
        markWidget->set_time(2,3313,value);
    }
    else if(le == le_setDiamondT){
        markWidget->set_time(3,3314,value);
    }
}

void WatchDiamondWidget::set_var_params(double *values)
{
    markWidget->set_var_params(values);
}

void WatchDiamondWidget::set_var_param(int varNum, double value)
{
    markWidget->set_var_param(varNum,value);
}

void WatchDiamondWidget::open_project()
{
    markWidget->open_project();
}

void WatchDiamondWidget::new_project()
{
    markWidget->new_project();
}

void WatchDiamondWidget::hal_config()
{
    int val=system("wish /usr/share/emc/tcl/bin/halshow.tcl &");
    if(val)
        printf("open halshow fail\n");
}

void WatchDiamondWidget::hal_meter()
{
    int val=system("halmeter &");
    if(val)
        printf("open halmeter fail\n");
}

void WatchDiamondWidget::hal_scope()
{
    int val=system("halscope &");
    if(val)
        printf("open halscope fail\n");
}

void WatchDiamondWidget::setup_algorithm()
{
//    setupDialog->set_dt_threshold(markWidget->get_dt_threshold());
//    setupDialog->set_dt_pix_num_differ(markWidget->get_dt_pix_num_differ());
//    double value = markWidget->get_dt_search_region_width();
//    printf("Width:%f\n",value);
//    setupDialog->set_dt_search_region_width(value);

//    if(setupDialog->exec() == QDialog::Accepted){
//        printf("test\n");
//        int type = setupDialog->get_algorithm_type();
//        markWidget->set_diamond_detect_algorithm(type);
//        markWidget->set_dt_threshold(setupDialog->get_dt_threshold());
//        markWidget->set_dt_pix_num_differ(setupDialog->get_dt_pix_num_differ());
//        markWidget->set_dt_search_region_width(setupDialog->get_dt_search_region_width());
//    }
}

void WatchDiamondWidget::clear_error_message()
{
    te_sysMessage->clear();
}


void WatchDiamondWidget::micro_adjust_offset(double value)
{
    int index=0;
    QDoubleSpinBox* sp = qobject_cast<QDoubleSpinBox*>(sender());
    if(sp == sp_pickupOffsetX){
        index = 0;
    }
    else if(sp == sp_pickupOffsetY){
        index = 1;
    }
    else if(sp == sp_glueOffsetX){
        index =2;
    }
    else if(sp == sp_glueOffsetY){
        index =3;
    }
    markWidget->set_offset(index,value);

}

void WatchDiamondWidget::set_diamond_distance(double value)
{
    QDoubleSpinBox* sp = qobject_cast<QDoubleSpinBox*>(sender());
    if(sp == sp_distance){
        markWidget->set_distance_between_diamonds(value);
    }
}
