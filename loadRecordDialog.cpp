#include "loadRecordDialog.h"
#include <sys/types.h>
#include <dirent.h>
#include <QMessageBox>
#include <stdio.h>

LoadRecordDialog::LoadRecordDialog(QWidget* parent):QDialog(parent){
    setupUi(this);
    connect(bt_delete,SIGNAL(clicked()), this, SLOT(delete_record()));
}

void LoadRecordDialog::set_combo_box(const char *currentProject){
    strcpy(currentPrj, currentProject);
    set_combo_box();
}

void LoadRecordDialog::delete_record(){
    int r;
    QString prjName=cb_records->currentText();
    if(prjName.size()==0)
        return;
    QString msg= QString::fromUtf8("是否删除定位存档：\n  ")+prjName;
    r=QMessageBox::information(this,QString::fromUtf8("删除确认"),msg,
                             QMessageBox::Yes|QMessageBox::No);
    if(r==QMessageBox::Yes){
        char sh[256];
        sprintf(sh, "rm -rf \"/home/u/cnc/CCD定位存档/%s\"", prjName.toUtf8().constData());
        if(system(sh))
            printf(" ERROR: %s fail\n", sh);
        set_combo_box();
    }
}

void LoadRecordDialog::set_combo_box(){
    cb_records->clear();
    DIR* dir=opendir("/home/u/cnc/镶钻存档/");
    if(dir){
        struct dirent* content;
        while((content=readdir(dir))!=NULL){
            if(strcmp(content->d_name, ".")!=0 && strcmp(content->d_name, "..")!=0
                    && strcmp(content->d_name, currentPrj)){
                cb_records->addItem(QString::fromUtf8(content->d_name));
            }
        }
        closedir(dir);
    }
}

QString LoadRecordDialog::get_project_name()const{
    return cb_records->currentText();
}
