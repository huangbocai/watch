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

private slots:
    void choose_algorithm();
    
private:
    int algorithmType; //0: DT; 1: Hough

};

#endif // ALGORITHMDIALOG_H
