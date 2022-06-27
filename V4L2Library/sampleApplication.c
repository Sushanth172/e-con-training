#include <stdio.h>
#include "v4l2dev.h"
#include <libudev.h>
#include <libusb-1.0/libusb.h>
int main()
{
  int count=0,indexFromUser,retVal=0;
  char devicePath[100],serialNumber[100], deviceName[100];
  char productId[100], vendorId[100];
  count = getDeviceCount();

  printf("\nNUMBER OF V4L2 DEVICES CONNECTED:%d",count);
  if(count == 0)
  {
    return 0;
  }
  printf("\nENTER THE DEVICE INDEX:");
  scanf("%d",&indexFromUser);

  if(indexFromUser <= count && indexFromUser != 0)
  {
    getDeviceInfo(serialNumber,deviceName,productId,vendorId,devicePath,indexFromUser);
    printf("\nDEVICE INDEX:%d\nSERIAL NO   :%s\nDEVICE NAME :%s\nPRODUCT ID  :%s\nVENDOR ID   :%s\nDEVICE PATH :%s\n",indexFromUser,serialNumber,deviceName,productId,vendorId,devicePath);

    printf("\nENTER THE INDEX OF THE DEVICE TO OPEN:");
    scanf("%d",&indexFromUser);
    //OPENING THE FILE BY CALLING THE OPENFILE FUNCTION...PASSING THE INDEX 
    retVal = openDevice(indexFromUser);
    
    if(retVal<0)
    	printf("ERROR IN OPENING THE FILE...");
    else
    {
        printf("FILE IS OPENED");
    }
    getDeviceFormat();
  }
  else
  {
    printf("\nINVALID INPUT!!\n");
  }
  return 0;
}
