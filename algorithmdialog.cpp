#include "algorithmdialog.h"
#include "ui_algorithmdialog.h"

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
