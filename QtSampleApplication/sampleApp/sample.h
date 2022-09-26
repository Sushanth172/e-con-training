#ifndef SAMPLE_H
#define SAMPLE_H

#include <QDebug>
#include </home/nivedha/v4l2dev.h>
#include <QStringListModel>
#include <unistd.h>

#define MAX_CHAR 100

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
    char devicePath[MAX_CHAR],serialNumber[MAX_CHAR], deviceName[MAX_CHAR];
    char productId[MAX_CHAR], vendorId[MAX_CHAR];
    int bytesused;
    unsigned char *buffer=NULL;
    QStringList deviceNameList;

public slots:
        void deviceEnumeration();
        void selectDevice(int deviceIndex);
};

#endif // SAMPLE_H
