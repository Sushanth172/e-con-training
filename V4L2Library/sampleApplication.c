#include <stdio.h>
#include "v4l2dev.h"
#include <libudev.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#define MAX_CHAR 100
#define PASS 1
#define FAIL -1

int main()
{
  int count=0,indexFromUser,retVal=0;
  char devicePath[MAX_CHAR],serialNumber[MAX_CHAR], deviceName[MAX_CHAR];
  char productId[MAX_CHAR], vendorId[MAX_CHAR];
  int formatCount,formatIndexFromUser;
  int frame_height,frame_width;
  char pixelFormat[MAX_CHAR]; //REQUIRED DECLARATIONS FOR GET DEVICE FORMAT
  int width,height; //REQUIRED DECLARATIONS FOR ENUM FORMAT
  int formats=0;
  unsigned char *buffer = (unsigned char*)malloc(640*480);

  //TO GET NUMBER OF DEVICES CONNECTED
  getDeviceCount(&count);

  //GET FORMAT TYPE
  int fps;
  char formatType[MAX_CHAR];
  //PRINTING THE NUMBER OF DEVICES CONNECTED
  printf("\nNUMBER OF V4L2 DEVICES CONNECTED:%d",count);
  if(count == 0)
  {
    return FAIL;
  }
  //GETTING THE DEVICE INDEX FROM USER TO DISPLAY RESPECTIVE DEVICE' DATA
  printf("\nENTER THE DEVICE INDEX:");
  scanf("%d",&indexFromUser);

  //VALIDATING THE INDEX FROM USER
  if(indexFromUser <= count && indexFromUser != 0)
  {
    //CALLING API TO GET DEVICE INFO
    getDeviceInfo(serialNumber,deviceName,productId,vendorId,devicePath,indexFromUser);
    printf("\n-------------------DEVICE %d DATA-----------------------",indexFromUser);
    printf("\nDEVICE INDEX:%d\nSERIAL NO   :%s\nDEVICE NAME :%s\nPRODUCT ID  :%s\nVENDOR ID   :%s\nDEVICE PATH :%s\n",indexFromUser,serialNumber,deviceName,productId,vendorId,devicePath);
    printf("\n--------------------------------------------------------\n");

    //GETTING DEVICE INDEX FROM USER TO OPEN THE DEVICE
    printf("\nENTER THE INDEX OF THE DEVICE TO OPEN:");
    scanf("%d",&indexFromUser);

    //VALIDATING THE INDEX
    if(indexFromUser <= count && indexFromUser != 0)
    {
      printf("\nDEVICE %d IS OPENED",indexFromUser);
    }
    else
    {
      printf("\nINVALID DEVICE INDEX TO OPEN!!\n");
      return FAIL;
    }
    //OPENING THE DEVICE BY CALLING THE OPENFILE FUNCTION...PASSING THE INDEX
    retVal = openDevice(indexFromUser);

    if(retVal<=0)
      printf("ERROR IN OPENING THE DEVICE...");
    else
    {
      printf("\n-------------------------------------DEVICE %d CURRENT FORMAT-------------------------------------------",indexFromUser);
    }
    //API TO GET CURRENT FORMAT OF THE DEVICE
    getCurrentFormat(&frame_height,&frame_width,pixelFormat);
    printf("\nHEIGHT       :%d\n",frame_height);
    printf("WIDTH        :%d\n",frame_width);
    printf("PIXEL FORMAT :%s\n",pixelFormat);

    //API TO COUNT THE NUMBER OF FORMATS SUPPORTED BY THE DEVICE
    getFormats(&formats);
    printf("\nTotal Formats:%d\n",formats );

    printf("\n---------------------------PIXEL FORMATS AVAILABLE IN THE DEVICE---------------------------------------");

    /* ITERATE THE TOTAL FORMATS ONE BY ONE AND CALLING THE API
       WHICH DISPLAYS ALL THE FORMATS SUPPORTED BY THE DEVICE */
    for(int formatIterator=0;formatIterator<formats;formatIterator++)
    {
      getFormatType(formatIterator,formatType,&width,&height,&fps);
      printf("\n%d\tPIXEL FORMAT: %s\tHEIGHT: %d\tWIDTH: %d\tFRAMES PER SECOND : %d\t\n",formatIterator+1,formatType,height,width,fps);
      printf("-------------------------------------------------------------------------------------------------------");
    }
    //GETTING THE FORMAT INDEX FROM THE USER TO SET THAT FORMAT TO THE DEVICE
    printf("\nENTER THE FORMAT INDEX TO SET THE FORMAT:");
    scanf("%d",&formatIndexFromUser);
    if(formatIndexFromUser <= formats && formatIndexFromUser != 0)
    {
      setFormat(formatIndexFromUser);
    }
    //GETTING THE CURRENT FORMAT OF THE DEVICE AFTER THE FORMAT HAS BEEN SET BY getFormatType API
    getCurrentFormat(&frame_height,&frame_width,pixelFormat);
    printf("\n-------------------------------------DEVICE %d CURRENT FORMAT-------------------------------------------",indexFromUser);
    printf("\nHEIGHT       :%d\n",frame_height);
    printf("WIDTH        :%d\n",frame_width);
    printf("PIXEL FORMAT :%s\n",pixelFormat);
    //VALIDATING GRAB FRAME
    if(grabFrame(buffer)!=PASS)
    {
      printf("\nGRAB FRAME IS FAILED!!");
      return FAIL;
    }
  }
  else
  {
    printf("\nINVALID DEVICE INDEX!!\n");
  }
  //CLOSING THE DEVICE
  closeDevice();
  return PASS;
}
