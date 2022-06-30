#include <stdio.h>
#include "v4l2dev.h"
#include <libudev.h>
#include <libusb-1.0/libusb.h>
int main()
{
  int count=0,indexFromUser,retVal=0;
  char devicePath[100],serialNumber[100], deviceName[100];
  char productId[100], vendorId[100];
  int formatCount,formatIndexFromUser;
  int frame_height,frame_width,bytesPerLine; //REQUIRED DECLARATIONS FOR GET DEVICE FORMAT
  int width,height; //REQUIRED DECLARATIONS FOR ENUM FORMAT
  char *description;
  int formats=0;
  //TO GET NUMBER OF DEVICES CONNECTED
  getDeviceCount(&count);
  //GET FORMAT TYPE
  int fps;
  char formatType[100];
  printf("\nNUMBER OF V4L2 DEVICES CONNECTED:%d",count);
  if(count == 0)
  {
    return -1;
  }
  printf("\nENTER THE DEVICE INDEX:");
  scanf("%d",&indexFromUser);

  if(indexFromUser <= count && indexFromUser != 0)
  {
    getDeviceInfo(serialNumber,deviceName,productId,vendorId,devicePath,indexFromUser);
    printf("\n-------------------DEVICE %d DATA-----------------------",indexFromUser);
    printf("\nDEVICE INDEX:%d\nSERIAL NO   :%s\nDEVICE NAME :%s\nPRODUCT ID  :%s\nVENDOR ID   :%s\nDEVICE PATH :%s\n",indexFromUser,serialNumber,deviceName,productId,vendorId,devicePath);
    printf("\n--------------------------------------------------------\n");
    printf("\nENTER THE INDEX OF THE DEVICE TO OPEN:");
    scanf("%d",&indexFromUser);
    //OPENING THE FILE BY CALLING THE OPENFILE FUNCTION...PASSING THE INDEX
    retVal = openDevice(indexFromUser);

    if(retVal<0)
    printf("ERROR IN OPENING THE DEVICE...");
    else
    {
      printf("\n----------------------------------------DEVICE %d FORMAT-----------------------------------------------",indexFromUser);
    }

    getCurrentFormat(&frame_height,&frame_width,&bytesPerLine);
    printf("\nHEIGHT         :%d\n",frame_height);
    printf("WIDTH          :%d\n",frame_width);
    printf("BYTES PER LINE :%d\n",bytesPerLine);

    getFormats(&formats);
    printf("\nTotal Formats:%d\n",formats );

    printf("\n---------------------------PIXEL FORMATS AVAILABLE IN THE DEVICE---------------------------------------");
    for(int formatIterator=0;formatIterator<formats;formatIterator++)
    {
      getFormatType(formatIterator,formatType,&width,&height,&fps);
      printf("\n%d\tPIXEL FORMAT: %s\tHEIGHT: %d\tWIDTH: %d\tFRAMES PER SECOND : %d\t\n",formatIterator+1,formatType,height,width,fps);
      printf("-------------------------------------------------------------------------------------------------------");
    }


    /*formatCount=enumFormat(description,&framePerSecond,&width,&height);
    printf("\n%d\tDESCRIPTION: %s\tHEIGHT: %d\tWIDTH: %d\tFRAMES PER SECOND : %d\t\n",formatIndex,formatDescriptor.description,frameIntervalValue.height,frameIntervalValue.width,framePerSecond);*/

    printf("\nENTER THE FORMAT INDEX TO SET THE FORMAT:");
    scanf("%d",&formatIndexFromUser);
    if(formatIndexFromUser <= count && formatIndexFromUser != 0)
    {
      setFormat(formatIndexFromUser);
    }
  }
  else
  {
    printf("\nINVALID DEVICE INDEX!!\n");
  }

  closeDevice();
  return 0;
}
