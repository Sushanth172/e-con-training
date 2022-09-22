#include <stdio.h>
#include "v4l2dev.h"
#include <libudev.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <libusb-1.0/libusb.h>
#include <opencv2/opencv.hpp>

#define MAX_CHAR 100
#define MILLISECONDS 1000
#define PASS 1
#define FAIL -1

pthread_t streamingThread;
pthread_mutex_t streamingMutex;
cv::Mat grabbedImage;
bool running = true;
int fps;
int currentFrameHeight,currentFrameWidth;
char currentPixelFormat[MAX_CHAR];
int bytesused=0;  //USED TO STORE THE BYTES USED IN QUEUE
unsigned char *buffer=NULL;

/*
THIS IS THE FUNCTION WHICH IS LOCKED BY MUTEX LOCKING MECHANISM
AND IS CALLED INFINITELY IN STARTSTREAM FUNCTION WHICH RUNS ON
SEPERATE THREAD
*/
int streaming()
{
  pthread_mutex_init(&streamingMutex,NULL);
  pthread_mutex_lock(&streamingMutex);

  //AFTER ONE DEQUEUE-QUEUE PROCESS COMPLETED...CALLING THE GRABFRAME API
  //INORDER TO GRAB THE FRAME FROM THE RENDERING DEVICE
  buffer=grabFrame(&bytesused);

  //VALIDATING WHEATHER THE BUFFER IS SUCCESSFULLY ALLOCATED MEMORY OR NOT
  if(buffer==NULL)
  {
    printf("\nMEMORY NOT ALLOCATED IN BUFFER IN GRAB FRAME\n");
    return FAIL;
  }
  //COPYING THE BUFFER DATA INTO THE GRABBED IMAGE DATA
  memcpy(grabbedImage.data,buffer,bytesused);
  //printf("\ninside streaming.................\n");

  //FREE THE BUFFER MEMORY EVERYTIME THE MEMCPY IS OVER
  if(buffer!=NULL)
  {
    free(buffer);
    buffer=NULL;
  }
  //CREATING ANOTHER MAT OBJECT
  cv::Mat rgbFrame;

  //CONVERTING GRABBED IMAGE(UYVY) INTO BGR FORMAT USING CVTCOLOR
  cv::cvtColor(grabbedImage, rgbFrame, cv::COLOR_YUV2BGR_UYVY);

  cv::namedWindow("STREAMING1", cv::WINDOW_AUTOSIZE);
  //USING IMSHOW...DISPLAYING THE CONVERTED FRAME IN THE WINDOW
  cv::imshow("STREAMING1",rgbFrame);

  //SETTING THE WINDOW TO VISIBLE FOR RESPECTIVE FRAMES PER SECONDS
  cv::waitKey(MILLISECONDS/fps);
  //printf("Start Streaming....\n");
  pthread_mutex_unlock(&streamingMutex);
  return PASS;
}

//THIS IS THE FUNCTION OF VOID POINTER TYPE FOR THREADING
void *startStream(void* arg)
{
  while(running)
  {
    if(streaming()!=PASS)
    {
      printf("\nFAILED TO STREAM\n");
      return (void*)FAIL;
    }
  }
}

int main()
{
  int count=0,indexFromUser,retVal=0;
  char devicePath[MAX_CHAR],serialNumber[MAX_CHAR], deviceName[MAX_CHAR];
  char productId[MAX_CHAR], vendorId[MAX_CHAR];
  int formatCount,formatIndexFromUser;
  char pixelFormat[MAX_CHAR]; //REQUIRED DECLARATIONS FOR GET DEVICE FORMAT
  int frame_height,frame_width;
  int width,height; //REQUIRED DECLARATIONS FOR ENUM FORMAT
  int formats=0,exit=1,exit1;

  //DECLARATIONS FOR UVC SETTINGS
  int setBrightnessValue;
  int minimum,maximum,steppingDelta,currentValue,defaultValue;
  int option;

  //TO GET NUMBER OF DEVICES CONNECTED
  getDeviceCount(&count);

  //GET FORMAT TYPE
  char formatType[MAX_CHAR];

  //PRINTING THE NUMBER OF DEVICES CONNECTED
  printf("\nNUMBER OF V4L2 DEVICES CONNECTED:%d",count);
  if(count == 0)
  {
    printf("\nNO DEVICE IS CONNECTED...\n");
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
    {
      printf("ERROR IN OPENING THE DEVICE...");
      return FAIL;
    }
    else
    {
      printf("\n-------------------------------------DEVICE %d CURRENT FORMAT-------------------------------------------",indexFromUser);
    }

    //API TO GET CURRENT FORMAT OF THE DEVICE
    getCurrentFormat(&frame_height,&frame_width,pixelFormat);
    printf("\nHEIGHT       :%d\n",frame_height);
    printf("WIDTH        :%d\n",frame_width);
    printf("PIXEL FORMAT :%s\n",pixelFormat);

    currentFrameHeight=frame_height;
    currentFrameWidth=frame_width;

    //CREATING MAT OBJECT
    grabbedImage = cv::Mat(currentFrameHeight, currentFrameWidth,CV_8UC2);

    //WAITING FOR DEQUEUE THREAD TO START
    sleep(3);
    pthread_create(&streamingThread,NULL,startStream,NULL);

    //API TO COUNT THE NUMBER OF FORMATS SUPPORTED BY THE DEVICE
    getFormats(&formats);
    printf("\nTOTAL FORMATS SUPPORTED BY THE DEVICE :%d\n",formats);

    printf("\n---------------------------PIXEL FORMATS AVAILABLE IN THE DEVICE---------------------------------------");

    /* ITERATE THE TOTAL FORMATS ONE BY ONE AND CALLING THE API
    WHICH DISPLAYS ALL THE FORMATS SUPPORTED BY THE DEVICE */
    for(int formatIterator=0;formatIterator<formats;formatIterator++)
    {
      getFormatType(formatIterator,formatType,&width,&height,&fps);
      printf("\n%d\tPIXEL FORMAT: %s\tWIDTH: %d\tHEIGHT: %d\tFRAMES PER SECOND : %d\t\n",formatIterator+1,formatType,width,height,fps);
      printf("-------------------------------------------------------------------------------------------------------");
    }

    printf("\nPRESS 1 IF YOU WANT TO ALTER DEVICE SETTINGS ");
    scanf("%d",&exit1);

    if(exit1==1)
    {
      running = false;
      pthread_join(streamingThread,NULL);
      pthread_cancel(streamingThread);
      pthread_mutex_destroy(&streamingMutex);
      cv::destroyWindow("STREAMING1");

      printf("\nCHOOSE 1 TO CHANGE THE FORMAT OF THE DEVICE\n");
      printf("\nCHOOSE 2 TO SET BRIGHTNESS OF THE DEVICE\n");

      printf("\nENTER YOUR OPTION: \n");
      scanf("%d",&option);
      switch(option)
      {
        case 1:
          //GETTING THE FORMAT INDEX FROM THE USER TO SET THAT FORMAT TO THE DEVICE
          printf("\nENTER THE FORMAT INDEX TO SET THE FORMAT: ");
          scanf("%d",&formatIndexFromUser);
          if(formatIndexFromUser <= formats && formatIndexFromUser != 0)
          {
            setFormat(formatIndexFromUser);
          }
          break;
        case 2:

          /*
             V4L2_CID_CONTRAST
             V4L2_CID_SATURATION
             V4L2_CID_HUE
             V4L2_CID_GAIN
             V4L2_CID_EXPOSURE_AUTO
             V4L2_CID_EXPOSURE_ABSOLUTE
             V4L2_CID_FOCUS_AUTO
             V4L2_CID_FOCUS_ABSOLUTE
             V4L2_CID_SHARPNESS
             V4L2_CID_GAMMA
             V4L2_CID_WHITE_BALANCE_TEMPERATURE
             V4L2_CID_BACKLIGHT_COMPENSATION
             V4L2_CID_ZOOM_ABSOLUTE
             V4L2_CID_PAN_ABSOLUTE
             V4L2_CID_TILT_ABSOLUTE
             V4L2_CID_AUTO_WHITE_BALANCE
          */
          if(getUVCControls(&minimum, &maximum, &steppingDelta, &currentValue, &defaultValue)!=PASS)
          {
            printf("\nGET BRIGHTNESS FAILED\n");
            return FAIL;
          }
          printf("\nMIN      :%d\n",minimum);
          printf("MAX      :%d\n",maximum);
          printf("STEP     :%d\n",steppingDelta);
          printf("CURRENT  :%d\n",currentValue);
          printf("DEFAULT  :%d\n",defaultValue);

          printf("\nENTER THE VALUE TO SET BRIGHTNESS:");
          scanf("%d",&setBrightnessValue);
          if(setUVCControls(setBrightnessValue)!=PASS)
          {
            printf("\nSET BRIGHTNESS FAILED\n");
            return FAIL;
          }
          break;
        default:
          printf("\nDEVICE STREAMS WITHOUT ANY CHANGES\n");
      }
    }
    else
    {
      printf("\nDEVICE RUNS ON DEFAULT FORMAT\n");
      return PASS;
    }

    //GETTING THE CURRENT FORMAT OF THE DEVICE AFTER THE FORMAT HAS BEEN SET BY getFormatType API
    getCurrentFormat(&currentFrameHeight,&currentFrameWidth,currentPixelFormat);
    printf("\n-------------------------------------DEVICE %d CURRENT FORMAT-------------------------------------------",indexFromUser);
    printf("\nHEIGHT       :%d\n",currentFrameHeight);
    printf("WIDTH        :%d\n",currentFrameWidth);
    printf("PIXEL FORMAT :%s\n",currentPixelFormat);

    //VALIDATION IF THE MAT OBJECT IS EMPTY OR NOT....
    //IF NOT EMPTY, RELEASE IT AND ALLOCATING THE SIZE
    if(grabbedImage.empty()==true)
    {
      //CREATING MAT OBJECT
      grabbedImage = cv::Mat(currentFrameHeight, currentFrameWidth,CV_8UC2);
    }
    else
    {
      grabbedImage.release();
      grabbedImage = cv::Mat(currentFrameHeight, currentFrameWidth,CV_8UC2);
    }

    //MAKE ONE SEC WAIT FOR ATLEAST ONE DEQUEUE-QUEUE PROCESS TO COMPLETE
    sleep(3);

    /*
    RUNNING IS FALSE WHEN SETTING THE FORMAT FOR THE DEVICE
    IN ORDER TO STREAM AGAIN AFTER THE FORMAT SET...RUNNING=TRUE
    */
    running=true;
    pthread_create(&streamingThread,NULL,startStream,NULL);

    printf("\nPRESS 0 TO STOP STREAMING\n");
    scanf("%d",&exit);

    if(exit==0)
    {
      //STOP THE THREAD(IMSHOW)
      running = false;
      pthread_join(streamingThread,NULL);
      pthread_cancel(streamingThread);
      pthread_mutex_destroy(&streamingMutex);
      cv::destroyWindow("STREAMING1");

      //CLOSING THE DEVICE
      closeDevice();
    }
  }
  else
  {
    printf("\nINVALID DEVICE INDEX!!\n");
  }
  return PASS;
}
