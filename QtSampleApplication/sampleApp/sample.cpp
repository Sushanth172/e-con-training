#include "sample.h"
QStringListModel Sample::deviceNameModel;
QStringListModel Sample::formatModel;
QStringListModel Sample::resolutionModel;
QStringListModel Sample::fpsModel;

Sample::Sample()
{
    number = 10;
    DeviceName="";
    pId ="";
    vId ="";
    devicePath="";
    sno="";
    DeviceName = (char*) malloc(20);
    sno = (char*) malloc(20);
    pId = (char*) malloc(20);
    vId = (char*) malloc(20);
    devicePath = (char*) malloc(20);
}

Sample::~Sample()
{
//    qDebug() << Q_FUNC_INFO;
   free(sno);
   free(DeviceName);
   free(pId);
   free(vId);
   free(devicePath);

}

void Sample::callFunc(){
    int iterator;
    deviceName.clear();
//  qDebug() << Q_FUNC_INFO;
    getDeviceCount(&number);
    qDebug() << number;

    for(iterator=0;iterator<number;iterator++)
    {
        getDeviceInfo(number,sno,DeviceName,pId,vId,devicePath);
        deviceName << DeviceName;
    }
    deviceNameModel.setStringList(deviceName);

}
void Sample::selectDevice(index){
    openDevice(index);
}
