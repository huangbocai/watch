
#include "diamodWidget.h"
#include "markWidget.h"


DiamodWidget::DiamodWidget(int argc, char **argv, QWidget *parent):QWidget(parent){
    widget=new MarkWidget(argc, argv,this);
}

DiamodWidget::~DiamodWidget(){
    delete widget;
}

const char* DiamodWidget::get_version(){
    return widget->get_version();
}

const char* DiamodWidget::get_current_project()const{
    return widget->get_current_project();
}
