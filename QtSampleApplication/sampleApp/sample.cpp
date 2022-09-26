#include "sample.h"
#define DEFAULT 1
QStringListModel Sample::deviceNameModel;
QStringListModel Sample::formatModel;
QStringListModel Sample::resolutionModel;
QStringListModel Sample::fpsModel;

Sample::Sample()
{
    number = 10;
    bytesused=10;
    deviceFormats=10;
    width=10;
    height=10;
    fps=10;
    buffer=(unsigned char*) malloc(500000);
}

Sample::~Sample()
{
    //    qDebug() << Q_FUNC_INFO;
}

void Sample::deviceEnumeration(){
    int iterator;
    deviceNameList.clear();
    deviceNameList.append("Select Device");
    //  qDebug() << Q_FUNC_INFO;
    getDeviceCount(&number);
    for(iterator=1;iterator<=number;iterator++)
    {
        getDeviceInfo(iterator,serialNumber,deviceName,productId,vendorId,devicePath);
        qDebug() << iterator << " " << serialNumber << " " << deviceName << " " << productId << " " << vendorId << " " << devicePath;
        deviceNameList << deviceName;
    }
    deviceNameModel.setStringList(deviceNameList);

}
void Sample::selectDevice(int index){

    if(index>0)
    {
        qDebug()<<"Index from comboBox:"<<index;
        openDevice(index);
        buffer=grabFrame(&bytesused);
        getFormatType(index,"UYVY",&width,&height,&fps);
//        getFormatType(formatIterator,formatType,&width,&height,&fps);


//        getFormats(&deviceFormats);
//        formatsList << deviceFormats;
//        formatModel.setStringList(formatsList);

    }
}
//grabFrame(&bytesused);
//int fwriteValidation=1;

//buffer=grabFrame(&bytesused);
//sleep(10);
//buffer=grabFrame(&bytesused);
//qDebug()<<"Used"<<bytesused;
//qDebug()<<"index:"<<index;
//        if(buffer==NULL)
//        {
//            printf("\nMEMORY NOT ALLOCATED IN BUFFER IN GRAB FRAME\n");
//        }
//        FILE *filePtr = NULL;
//        filePtr = fopen("/home/nivedha/qtSample.raw","wb");
//        if(filePtr== NULL)
//        {
//            printf("This %s file could not open....","qtSample.raw");
//        }
//        fwriteValidation=fwrite(buffer,bytesused,1,filePtr);

//        if(fwriteValidation==bytesused)
//        {
//            printf("Success");
//            fclose(filePtr);
//            return;
//        }
//        printf("Failed");

//        fclose(filePtr);
