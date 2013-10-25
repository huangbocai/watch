#ifndef WATCHDIAMONDWIDGET_H
#define WATCHDIAMONDWIDGET_H
#include "ui_watchdiamondwidget.h"
#include <QMainWindow>
#include <stdio.h>
#include "markWidget.h"
#include "algorithmdialog.h"

namespace Ui {
class WatchDiamondWidget;
}

class Coordinate
{
public:
    Coordinate(QLabel* lbx, QLabel* lby, QLabel* lbz, QLabel* lba, QLabel* lbb)
    {
        lb[0] = lbx;
        lb[1] = lby;
        lb[2] = lbz;
        lb[3] = lba;
        lb[4] = lbb;
    }

    void set_value(int index, double Val, int color){
        char buf[64];
        QString colorStyle[3] = {"color:red","color:Fuchsia","color:black"};
        //sprintf(buf,"<font color=red>%.3f</font>",Val);
        sprintf(buf,"%.3f",Val);
        lb[index]->setText(QString(buf));
        lb[index]->setStyleSheet(colorStyle[color]);

    }

private:
    QLabel* lb[5];
};

class WatchDiamondWidget : public QMainWindow, public Ui::WatchDiamondWidget
{
    Q_OBJECT
    
public:
    explicit WatchDiamondWidget(int argc, char **argv,QWidget *parent = 0);
    ~WatchDiamondWidget();

private slots:
    void update_emc_slot(const MarkEmcStatus& status);
    void update_infor_slot(const Information& infor);
    void set_axis(const double* val, int *colors);
    void machine_open_toggled(bool checked);
    void home();
    void ask_home();
    void zero();
    void change_vel_toggled(bool checked);
    void set_velocity(int value);
    //void finish_set_velocity();
    void jog();
    void end_jog();
    void io_button_toggled(bool checked);

    void scan_diamond();
    void scan_watch(bool checked);
    void set_glue(bool checked);
    void set_diamond(bool checked);

    void auto_run(bool checked);
    void pause(bool checked);
    void stop();
    void set_height();
    void set_time();
    void set_var_param(int varNum, double value=0);

    void micro_adjust_offset(double value);


    //menu action
    void open_project();
    void save_as_project();
    void hal_config();
    void hal_meter();
    void hal_scope();
    void setup_algorithm();

    void clear_error_message();

protected:
    void keyPressEvent(QKeyEvent * event);
    void keyReleaseEvent(QKeyEvent * event);
    void closeEvent(QCloseEvent *event);


private:
    QString double_to_qstring(double val){
        char buf[128];
        sprintf(buf,"%.3f",val);
        return QString(buf);
    }

    void io_function(int index, bool checked);

    MarkWidget* markWidget;
    Coordinate* coordinate;
    QDoubleValidator* doubleValidator;
    QPushButton* jogButtons[10];
    QPushButton* ioButtons[6];
    QTimer* timer;
    AlgorithmDialog* setupDialog;

    double fastVel;
    double slowVel;
    double currentVel;
    double axisPos[5];
    bool autoRun;
};

#endif // WATCHDIAMONDWIDGET_H
