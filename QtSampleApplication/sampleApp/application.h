#ifndef SAMPLE_H
#define SAMPLE_H

#include <QDebug>
#include <v4l2dev.h>
#include <QStringListModel>
#include <unistd.h>
#include <qquickitem.h>
#include "renderer.h"
#include "h264decoder.h"
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
    H264Decoder *H264_Decoder;
    bool MJPEG_flag = false;
    bool H264_flag = false;
    char devicePath[MAX_CHAR],serialNumber[MAX_CHAR], deviceName[MAX_CHAR];
    char productId[MAX_CHAR], vendorId[MAX_CHAR];
    int bytesused;
    unsigned char *buffer=NULL;
    unsigned char *decompressedBuffer=NULL;
    int stride;
    struct v4l2_format frmt;
    struct v4l2_streamparm strparm;

    QStringList deviceNameList,FormatList,resolution_List, fps_List;
    int width, height, fps,NoOfFormats;
    char formats;

    int comboBox_format_index=0,comboBox_res_index=0,comboBox_fps_index=0,initial_check_index;

    QFuture<void> future,decompressFuture;
    char format_type;
    void paint();
    QTimer *timer;
    bool renderFlag;

signals:
    void emitformats(int deviceIndex);
    void emitresolution(int deviceIndex);
    void emitfps(int deviceIndex);
    void devicedisconnected();

public slots:
        void deviceEnumeration();
        int selectDevice(int deviceIndex);
        int totalFormats(int deviceIndex);
        void enumFormat();
        void enumResolution(int resIndex);
        void enumFps(int fpsIndex);
//        void deviceDetails(int index1);
//        static void render(Application *app);
        void grabframe();
};

#endif // SAMPLE_H
