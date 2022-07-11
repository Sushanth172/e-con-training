#include <stdio.h>
#include "v4l2dev.h"
#include <libudev.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <libusb-1.0/libusb.h>

#include <opencv2/opencv.hpp>

#define MAX_CHAR 100
#define PASS 1
#define FAIL -1

pthread_t streamingThread;
pthread_mutex_t streamingMutex;

int currentFrameHeight,currentFrameWidth;
char currentPixelFormat[MAX_CHAR];
int bytesused=0;  //USED TO STORE THE BYTES USED IN QUEUE
unsigned char *buffer=NULL;

int streaming()
{
    pthread_mutex_init(&streamingMutex,NULL);
    pthread_mutex_lock(&streamingMutex);

    //CREATING MAT OBJECT
    cv::Mat grabbedImage = cv::Mat(currentFrameHeight, currentFrameWidth,CV_8UC2);

    //AFTER ONE DEQUEUE-QUEUE PROCESS COMPLETED...CALLING THE GRABFRAME API
    //INORDER TO GRAB THE FRAME FROM THE RENDERING DEVICE
    buffer=grabFrame(&bytesused);
    //VALIDATING WHEATHER THE BUFFER IS SUCCESSFULLY ALLOCATED MEMORY OR NOT
    if(buffer==NULL)
    {
      printf("\nMEMORY NOT ALLOCATED IN BUFFER IN GRAB FRAME\n");
    }

    //COPYING THE BUFFER DATA INTO THE GRABBED IMAGE DATA
    memcpy(grabbedImage.data,buffer,bytesused);
    free(buffer);
    //CREATING ANOTHER MAT OBJECT
    cv::Mat rgbFrame;

    //CONVERTING GRABBED IMAGE(UYVY) INTO BGR FORMAT USING CVTCOLOR
    cv::cvtColor(grabbedImage, rgbFrame, cv::COLOR_YUV2BGR_UYVY);

    //USING IMSHOW...DISPLAYING THE CONVERTED FRAME IN THE WINDOW
    cv::imshow("SUSHANTH' DEVICE",rgbFrame);

    //SETTING THE WINDOW TO VISIBLE FOR 500 MS
    cv::waitKey(60);
    pthread_mutex_unlock(&streamingMutex);
    return PASS;
}

//THIS IS THE FUNCTION OF VOID POINTER TYPE FOR THREADING
void *startStream(void* arg)
{
  printf("\nSUSHANTH!! YOUR DEVICE IS READY FOR STREAMING\n");
  sleep(3);
  for(;;)
  {
    if(streaming()!=PASS)
    {
      printf("\nDEQUEUE BUFFER FAILED\n");
      return (void*)FAIL;
    }
  }
}

int main()
{
  FILE *fptr= NULL;
  int count=0,indexFromUser,retVal=0;
  char devicePath[MAX_CHAR],serialNumber[MAX_CHAR], deviceName[MAX_CHAR];
  char productId[MAX_CHAR], vendorId[MAX_CHAR];
  int formatCount,formatIndexFromUser;
  char pixelFormat[MAX_CHAR]; //REQUIRED DECLARATIONS FOR GET DEVICE FORMAT
  int frame_height,frame_width;
  int width,height; //REQUIRED DECLARATIONS FOR ENUM FORMAT
  int formats=0;
  int fwrite_validation;

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
    getDeviceInfo(indexFromUser,serialNumber,deviceName,productId,vendorId,devicePath);
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
    printf("\nTotal Formats:%d\n",formats);

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
    getCurrentFormat(&currentFrameHeight,&currentFrameWidth,currentPixelFormat);
    printf("\n-------------------------------------DEVICE %d CURRENT FORMAT-------------------------------------------",indexFromUser);
    printf("\nHEIGHT       :%d\n",currentFrameHeight);
    printf("WIDTH        :%d\n",currentFrameWidth);
    printf("PIXEL FORMAT :%s\n",currentPixelFormat);

    //API TO START THE RENDERING AND TO CREATE THE THREAD OPERATION
    startRender();

    //MAKE ONE SEC WAIT FOR ATLEAST ONE DEQUEUE-QUEUE PROCESS TO COMPLETE
    sleep(3);
    pthread_create(&streamingThread,NULL,startStream,NULL);
  }
  else
  {
    printf("\nINVALID DEVICE INDEX!!\n");
  }

  //CLOSING THE DEVICE
  pthread_mutex_destroy(&streamingMutex);
  closeDevice();
  return PASS;
}