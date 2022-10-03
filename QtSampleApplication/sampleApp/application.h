#ifndef SAMPLE_H
#define SAMPLE_H

#include <QDebug>
#include <v4l2dev.h>
#include <QStringListModel>
#include <unistd.h>
#include <qquickitem.h>
#include "renderer.h"
#include <linux/videodev2.h>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

#define MAX_CHAR 100

class Application : public QQuickItem
{
  Q_OBJECT
  public:
    static QStringListModel deviceNameModel;
    static QStringListModel formatTypeModel;
    static QStringListModel formatAvailableModel;
    static QStringListModel resolutionModel;
    static QStringListModel fpsModel;

    Application();
    ~Application();
    int number;
    Renderer *m_renderer;
    bool MJPEG_flag = false;
    char devicePath[MAX_CHAR],serialNumber[MAX_CHAR], deviceName[MAX_CHAR];
    char productId[MAX_CHAR], vendorId[MAX_CHAR];
    int bytesused;
    unsigned char *buffer=NULL;
    unsigned char *decompressedBuffer=NULL;
    int stride;
    struct v4l2_format frmt;
    struct v4l2_streamparm strparm;

    QStringList deviceNameList;
    QStringList FormatList;
//    QStringList Form
//    int formats, width, height, fps;
    QFuture<void> future,decompressFuture;
    char format_type;
    void paint();
    QTimer *timer;
    bool renderFlag;

public slots:
        void deviceEnumeration();
        void selectDevice(int deviceIndex);
//        void formatType();
//        static void render(Application *app);
        void grabframe();
};

#endif // SAMPLE_H
