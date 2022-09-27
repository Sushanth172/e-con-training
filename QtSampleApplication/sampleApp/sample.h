#ifndef SAMPLE_H
#define SAMPLE_H

#include <QDebug>
#include <v4l2dev.h>
#include <QStringListModel>
#include <unistd.h>
#include <qquickitem.h>
#include "renderer.h"
#include <QTimer>

#define MAX_CHAR 100

class Sample : public QQuickItem
{
  Q_OBJECT
  public:
    static QStringListModel deviceNameModel;
    static QStringListModel formatModel;
    static QStringListModel resolutionModel;
    static QStringListModel fpsModel;

    Sample();
    ~Sample();
    int number;
    Renderer *m_renderer;
    bool MJPEG_flag = false;
    char devicePath[MAX_CHAR],serialNumber[MAX_CHAR], deviceName[MAX_CHAR];
    char productId[MAX_CHAR], vendorId[MAX_CHAR];
    int bytesused;
    unsigned char *buffer=NULL;
    QStringList deviceNameList;
    void paint();
    QTimer *timer;
    bool renderFlag;

public slots:
        void deviceEnumeration();
        void selectDevice(int deviceIndex);
        void grabframe();
};

#endif // SAMPLE_H
