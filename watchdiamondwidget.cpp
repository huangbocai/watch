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

    doubleValidator = new QDoubleValidator(this);
    le_setVelocity->setValidator(doubleValidator);

    slowVel = markWidget->get_slow_velocity();
    fastVel = markWidget->get_fast_velocity();
    currentVel = slowVel;
    QString vel = double_to_qstring(currentVel);
    le_setVelocity->setText(vel);
    autoRun = false;



    te_sysMessage->setTextColor(QColor(Qt::red));
    te_sysMessage->setReadOnly(true);
    te_sysMessage->setText(QString("system Message:\n"));

    jogButtons[0] = bt_xPlus; jogButtons[1] = bt_xReduce; jogButtons[2] = bt_yPlus; jogButtons[3] = bt_yReduce;
    jogButtons[4] = bt_zPlus; jogButtons[5] = bt_zReduce; jogButtons[6] = bt_aPlus; jogButtons[7] = bt_aReduce;
    jogButtons[8] = bt_bPlus; jogButtons[9] = bt_bReduce;

    ioButtons[0] = tb_1; ioButtons[1] = tb_2; ioButtons[2] = tb_3;
    ioButtons[3] = tb_4; ioButtons[4] = tb_5; ioButtons[5] = tb_6;

    connect(markWidget,SIGNAL(update_emc_status(MarkEmcStatus)),this,SLOT(update_emc_slot(MarkEmcStatus)));    
    connect(markWidget,SIGNAL(update_infor(Information)),this,SLOT(update_infor_slot(Information)));
    connect(bt_machineRun,SIGNAL(toggled(bool)),this,SLOT(machine_open_toggled(bool)));
    connect(bt_home,SIGNAL(clicked()),this,SLOT(home()));
    connect(bt_zero,SIGNAL(clicked()),this,SLOT(zero()));
    connect(bt_changeVelocity,SIGNAL(toggled(bool)),this,SLOT(change_vel_toggled(bool)));
    connect(le_setVelocity,SIGNAL(editingFinished()),this,SLOT(finish_set_velocity()));
    connect(bt_scanDiamond,SIGNAL(clicked()),this,SLOT(scan_diamond()));
    connect(bt_scanWatch,SIGNAL(clicked()),this,SLOT(scan_watch()));
    connect(bt_setGlue,SIGNAL(clicked()),this,SLOT(set_glue()));
    connect(bt_setDiamond,SIGNAL(clicked()),this,SLOT(set_diamond()));
    connect(action_halConfig,SIGNAL(triggered()),this,SLOT(hal_config()));
    connect(action_halMeter,SIGNAL(triggered()),this,SLOT(hal_meter()));
    connect(action_halScope,SIGNAL(triggered()),this,SLOT(hal_scope()));
    connect(bt_autoRun,SIGNAL(toggled(bool)),this,SLOT(auto_run(bool)));
    connect(bt_pause,SIGNAL(toggled(bool)),this,SLOT(pause(bool)));
    connect(bt_stop,SIGNAL(clicked()),this,SLOT(stop()));

    for(int i=0; i<10; i++){
        connect(jogButtons[i],SIGNAL(pressed()),this,SLOT(jog()));
        connect(jogButtons[i],SIGNAL(released()),this,SLOT(end_jog()));
    }

    for(int i=0; i<6; i++){
        connect(ioButtons[i],SIGNAL(toggled(bool)),this,SLOT(io_button_toggled(bool)));
    }

    bt_machineRun->setChecked(true);
}

WatchDiamondWidget::~WatchDiamondWidget()
{
    delete markWidget;
}

void WatchDiamondWidget::update_emc_slot(const MarkEmcStatus& status)
{
    set_axis(status.cmdAxis);
}

void WatchDiamondWidget::update_infor_slot(const Information& infor){
    char buf[12];
    sprintf(buf,"%d",infor.diamondNum);
    lb_diamondNum->setText(QString(buf));
    sprintf(buf,"%d/%d",infor.watchPosIndex,infor.watchPosNum);
    lb_watchIndex->setText(QString(buf));
    sprintf(buf,"%d/%d",infor.gluePosIndex,infor.holePosNum);
    lb_glueIndex->setText(QString(buf));
    sprintf(buf,"%d/%d",infor.holePosIndex,infor.holePosNum);
    lb_holesIndex->setText(QString(buf));
    if(autoRun && infor.endAutoRun){
        autoRun = false;
        if(bt_autoRun->isChecked())
            bt_autoRun->setChecked(false);
    }
}

void WatchDiamondWidget::set_axis(const double* val)
{
    for(int i=0 ; i<5; i++)
        coordinate->set_value(*(val+i),i);
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
    markWidget->home();
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
            button->setText(QString::fromUtf8("快速"));
            currentVel = fastVel;
            QString vel = double_to_qstring(currentVel);
            le_setVelocity->setText(vel);
        }
        else{
            button->setText(QString::fromUtf8("慢速"));
            currentVel = slowVel;
            QString vel = double_to_qstring(currentVel);
            le_setVelocity->setText(vel);
        }
    }
}

void WatchDiamondWidget::finish_set_velocity()
{
    if(bt_changeVelocity->isChecked()){
        QString str = le_setVelocity->text();
        fastVel = str.toDouble();
        currentVel = fastVel;
        markWidget->set_fast_velocity(fastVel);
    }
    else{
        QString str = le_setVelocity->text();
        slowVel = str.toDouble();
        currentVel = slowVel;
        markWidget->set_slow_velocity(slowVel);
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
        velocity *= (-1);
        break;
    case Qt::Key_Down:
        axis = 1;
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

void WatchDiamondWidget::scan_diamond()
{
    markWidget->scan_diamon();
}

void WatchDiamondWidget::scan_watch()
{
    markWidget->scan_watch();
}

void WatchDiamondWidget::set_glue()
{
    markWidget->set_glue();
}

void WatchDiamondWidget::set_diamond()
{
    markWidget->set_diamond();
}

void WatchDiamondWidget::auto_run(bool checked)
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if(button == bt_autoRun){
        if(checked){
            markWidget->auto_run();
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
        }
    }
}

void WatchDiamondWidget::stop()
{
    markWidget->stop();
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
