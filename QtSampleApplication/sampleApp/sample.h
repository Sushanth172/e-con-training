#ifndef SAMPLE_H
#define SAMPLE_H

#include <QDebug>
#include <v4l2dev.h>
#include <QStringListModel>

class Sample : public QObject
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
    char *DeviceName;
    char *sno;
    char *pId;
    char *vId;
    char *devicePath;
    QStringList deviceName;

public slots:
        void callFunc();
        void selectDevice(int deviceIndex);
};

#endif // SAMPLE_H
