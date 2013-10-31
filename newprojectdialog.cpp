#include "newprojectdialog.h"
#include <sys/types.h>
#include <dirent.h>
#include <iostream>
#include <QPushButton>
NewProjectDialog::NewProjectDialog(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    lb_warning->setVisible(false);
    buttonBox->button(QDialogButtonBox::Ok)->setText(QString::fromUtf8("确定"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(QString::fromUtf8("取消"));
    connect(le_projectName,SIGNAL(textChanged(QString)),this,SLOT(check(QString)));
}

NewProjectDialog::~NewProjectDialog()
{
    //delete ui;
}

void NewProjectDialog::check(QString projectName)
{
    //std::cout<<projectName.toUtf8().constData()<<std::endl;
    DIR* dir=opendir("/home/u/cnc/镶钻存档/");
    if(dir){
        struct dirent* content;
        while((content=readdir(dir))!=NULL){
            if(strcmp(content->d_name, projectName.toStdString().c_str())==0){
                lb_warning->setText(QString::fromUtf8("警告：该工程已存在！"));
                lb_warning->setVisible(true);
                buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
                break;
            }
            else{
                lb_warning->setVisible(false);
                buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            }
        }
        closedir(dir);
    }
}

QString NewProjectDialog::get_project_name()const
{
    return le_projectName->text();
}
