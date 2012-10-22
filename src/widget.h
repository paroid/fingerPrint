#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QThread>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>

#include "fvs.h"

class ProThread;

namespace Ui {
    class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private:
    Ui::Widget *ui;
    QString filename,filename1,filename2;
    ProThread *mainProThread,*matchThread1,*matchThread2;
private slots:
    void openFile();
    void processImage();
};


class ProThread:public QThread{
    Q_OBJECT
public:
    ProThread(QString,bool,double);
    void setPicLabel(QLabel *,QLabel *,QLabel *,QLabel *,QLabel *,QLabel *,QLabel *);
    void run();
private:
    bool genPic;
    QString bmpfilename;
    const static int defaultSetSize=1200;
    double radius;
    QLabel *originLabel,*directionLabel,*maskLabel,*enhanceLabel,*binarizeLabel,*thinningLabel,*minutiaLabel;
    FvsMinutiaSet_t minutiaSet;
};



#endif // WIDGET_H
