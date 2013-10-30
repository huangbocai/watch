#include "algorithmdialog.h"
#include "ui_algorithmdialog.h"
#include <stdio.h>

AlgorithmDialog::AlgorithmDialog(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    connect(rb_dt,SIGNAL(clicked()),this,SLOT(choose_algorithm()));
    connect(rb_hough,SIGNAL(clicked()),this,SLOT(choose_algorithm()));
    rb_dt->click();
}

AlgorithmDialog::~AlgorithmDialog()
{
    //delete ui;
}

void AlgorithmDialog::choose_algorithm()
{
    if(rb_dt == qobject_cast<QRadioButton*>(sender())){
        tw_setupParam->setCurrentIndex(0);
        algorithmType = 0;
    }
    else if(rb_hough == qobject_cast<QRadioButton*>(sender())){
        tw_setupParam->setCurrentIndex(1);
        algorithmType = 1;
    }

}

void AlgorithmDialog::set_dt_threshold(int value)
{
    sb_dtThreshold->setValue(value);
}

void AlgorithmDialog::set_dt_pix_num_differ(int value)
{
    sb_dtPixDiff->setValue(value);
}

int AlgorithmDialog::get_dt_threshold()
{
    return sb_dtThreshold->value();
}

int AlgorithmDialog::get_dt_pix_num_differ()
{
    return sb_dtPixDiff->value();
}

void AlgorithmDialog::set_dt_search_region_width(double value)
{
    printf("%f\n",value);
    sb_searchRegionW->setValue(value);
}

double AlgorithmDialog::get_dt_search_region_width()
{
    return sb_searchRegionW->value();
}
