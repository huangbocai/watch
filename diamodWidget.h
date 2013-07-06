#ifndef DIAMODWIDGET_H
#define DIAMODWIDGET_H

#include <QWidget>

class MarkWidget;

class DiamodWidget: public QWidget
{
    Q_OBJECT
public:
    DiamodWidget(int argc,  char **argv, QWidget* parent=0);
//    const QImage& get_view_image(int width, int height);
    const char* get_version();
    const char* get_current_project()const;
   ~DiamodWidget();

private:
    MarkWidget* widget;
};

#endif // DIAMODWIDGET_H
