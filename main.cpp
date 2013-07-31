#include <QApplication>
#include "diamodWidget.h"
#include "watchdiamondwidget.h"
int main(int argc, char** argv){
    QApplication app(argc, argv);
    //DiamodWidget* widget= new DiamodWidget(argc, argv);
    //widget->show();
    WatchDiamondWidget* widget = new WatchDiamondWidget(argc,argv);
    widget->show();

    return app.exec();
}
