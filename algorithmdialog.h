#ifndef ALGORITHMDIALOG_H
#define ALGORITHMDIALOG_H

#include <QDialog>
#include "ui_algorithmdialog.h"

namespace Ui {
class AlgorithmDialog;
}

class AlgorithmDialog : public QDialog, Ui_algorithmDialog
{
    Q_OBJECT
    
public:
    explicit AlgorithmDialog(QWidget *parent = 0);
    ~AlgorithmDialog();
    int get_algorithm_type(){return algorithmType;}
    void set_dt_threshold(int value);
    int get_dt_threshold();
    void set_dt_pix_num_differ(int value);
    int get_dt_pix_num_differ();
    void set_dt_search_region_width(double value);
    double get_dt_search_region_width();

private slots:
    void choose_algorithm();

    
private:
    int algorithmType; //0: DT; 1: Hough

};

#endif // ALGORITHMDIALOG_H
