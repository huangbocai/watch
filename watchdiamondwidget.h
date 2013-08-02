#ifndef WATCHDIAMONDWIDGET_H
#define WATCHDIAMONDWIDGET_H
#include "ui_watchdiamondwidget.h"
#include <QMainWindow>
#include <stdio.h>
#include "markWidget.h"

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

    void set_value(double Val, int index){
        char buf[64];
        sprintf(buf,"%.3f",Val);
        lb[index]->setText(QString(buf));
    }

private:
    QLabel* lb[5];
};
/*
class TextEditor : public QTextEdit
{
public:
    TextEditor(QTextEdit* parent=0):QTextEdit(parent){}
protected:
    void contextMenuEvent(QContextMenuEvent *e)
    {
        popupMenu = new QMenu(this);
        QAction* clear = new QAction(QString::fromUtf8("清除所有"),this);
        popupMenu->addAction(clear);
        popupMenu->popup(e->pos());
        popupMenu->exec();
    }
private:
    QMenu* popupMenu;

};
*/

class WatchDiamondWidget : public QMainWindow, public Ui::WatchDiamondWidget
{
    Q_OBJECT
    
public:
    explicit WatchDiamondWidget(int argc, char **argv,QWidget *parent = 0);
    ~WatchDiamondWidget();

private slots:
    void update_emc_slot(const MarkEmcStatus& status);
    void update_infor_slot(const Information& infor);
    void set_axis(const double* val);
    void machine_open_toggled(bool checked);
    void home();
    void zero();
    void change_vel_toggled(bool checked);
    void finish_set_velocity();
    void jog();
    void end_jog();
    void io_button_toggled(bool checked);

    void scan_diamond();
    void scan_watch();
    void set_glue();
    void set_diamond();

    void auto_run(bool checked);
    void pause(bool checked);
    void stop();

    //menu action
    void hal_config();
    void hal_meter();
    void hal_scope();

    void clear_error_message();

protected:
    void keyPressEvent(QKeyEvent * event);
    void keyReleaseEvent(QKeyEvent * event);


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
    //TextEditor* message;
    double fastVel;
    double slowVel;
    double currentVel;
    bool autoRun;
};

#endif // WATCHDIAMONDWIDGET_H
