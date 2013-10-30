#ifndef NEWPROJECTDIALOG_H
#define NEWPROJECTDIALOG_H

#include <QDialog>
#include "ui_newprojectdialog.h"

namespace Ui {
class NewProjectDialog;
}

class NewProjectDialog : public QDialog, Ui_NewProjectDialog
{
    Q_OBJECT
    
public:
    explicit NewProjectDialog(QWidget *parent = 0);
    ~NewProjectDialog();
    QString get_project_name()const;
private slots:
    void check(QString projectName);

private:
    //Ui::NewProjectDialog *ui;
    QPushButton* okButton;
};

#endif // NEWPROJECTDIALOG_H
