#include "v4l2dev.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <libudev.h>
#include <libusb-1.0/libusb.h>
#include <libv4l2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include <linux/uvcvideo.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#define PASS 1
#define FAIL -1
#define PIXEL_FORMAT 5
#define BUFFER_COUNT 3
#define CLEAR(x) memset(&(x), 0, sizeof(x))

struct buffer{
  void   *start;
  size_t  length;
};

FILE *fptr;
int fwrite_validation;

int bytesUsed=0;
int refIndex=0;
int dequeueIndex=0;
int fd=-1;
pthread_t dequeueThread;  //CREATING A THREAD ID
pthread_mutex_t mutex;
bool runningDequeue = true;

struct v4l2_buffer buf;
struct v4l2_requestbuffers req;
struct buffer *buffers = NULL;
unsigned char *temp_buffer = NULL;
struct v4l2_capability cam_cap;  //For getting capability of that device


int streamOff();
int xioctl(int fd,int request, void *arg);
void *render(void* arg);
int checkForValidNode(const char *dev_path);


#define fun m##a##i##n
int fun()
{
}
//SWITCHING OFF THE STREAM
int streamOff()
{
  //DESTROYING THE MUTEX
  //pthread_mutex_destroy(&mutex);
  if(fd < 0)
  {
    printf("\nFAILED IN OPENING THE DEVICE(STREAM OFF)\n");
    return FAIL;
  }
  if(xioctl(fd, VIDIOC_STREAMOFF, &buf.type)<0)
  {
    printf("STREAM OFF FAILED\n");
    return FAIL;
  }
  return PASS;
}

//DO IOCTL UNTILL THE RESOURCE IS AVAILABLE(RESOURCE TEMPORARILY UNAVAILABLE)
int xioctl(int fd,int request, void *arg)
{
  int resouceAvailableIndex,ioctlReturn;
  do {
    resouceAvailableIndex = ioctl(fd, request, arg);
  } while (resouceAvailableIndex == -1 && EAGAIN == errno);  //do ioctl until there is no EAGAIN error(resource temporarily unavailable, try again)
  return resouceAvailableIndex;
}

//CHECKING FOR VALID NODE USING THE DEVICE PATH
int checkForValidNode(const char *dev_path)
{
  int cam_fd;
  if ((cam_fd = open(dev_path, O_RDWR|O_NONBLOCK, 0)) < 0)
  {
    printf("Can't open camera device ");
    return FAIL;
  }

  //Check if the device is capable of streaming
  if(ioctl(cam_fd, VIDIOC_QUERYCAP, &cam_cap) < 0)
  {
    printf("VIDIOC_QUERYCAP failure");
    close(cam_fd);
    return FAIL;
  }
  close(cam_fd);

  //IF THE DEVICE CAPS FLAG IS ENABLED.....INVALID NODE
  if (cam_cap.device_caps & V4L2_CAP_META_CAPTURE)
  {
    return FAIL;
  }
  return PASS;
}

//GETTING THE DEVICE COUNT
int getDeviceCount(int *numberOfDevices)
{
  int count=0;
  struct udev *udev;
  struct udev_device *device,*parentdevice;
  struct udev_enumerate *enumerate;
  struct udev_list_entry *devices, *dev_list_entry;

  /* creating udev object */
  udev = udev_new();
  if (!udev)
  {
    printf("\nCannot create udev context.\n");
    return FAIL;
  }

  /* creating enumerate object */
  enumerate = udev_enumerate_new(udev);
  if (!enumerate)
  {
    printf("\nCannot create enumerate context.\n");
    return FAIL;
  }

  //SEARCHING V4L SUB_SYSTEM IN ENUMERATE
  udev_enumerate_add_match_subsystem(enumerate, "video4linux");

  //SEND ENUMERATE AS THE INPUT PARAMETER
  udev_enumerate_scan_devices(enumerate);

  //FILLING UP THE DEVICE LIST
  devices = udev_enumerate_get_list_entry(enumerate);

  //ITERATING ALL THE ENTITIES OF THE LIST
  udev_list_entry_foreach(dev_list_entry, devices)
  {
    const char *path=NULL,*sNo=NULL,*deviceName=NULL;
    const char *product_id, *vendor_id;

    path = udev_list_entry_get_name(dev_list_entry);

    //GETTING ALL THE DEVICES IN THE ABOVE PATH AND RETURN THE CHAR POINTER TO device
    device = udev_device_new_from_syspath(udev, path);//allocating a new udev device object
    const char *dev_path = udev_device_get_devnode(device);

    //CHECKING FOR TYPE AND SUB SYSTEM IN THE DEVICE AND STORE THE POINTER TO PARENT DEVICE
    parentdevice = udev_device_get_parent_with_subsystem_devtype(device,"usb","usb_device");
    if(!parentdevice)
    continue;
    vendor_id  =  udev_device_get_sysattr_value(parentdevice,"idVendor");

    product_id = udev_device_get_sysattr_value(parentdevice,"idProduct");

    sNo = udev_device_get_sysattr_value(parentdevice,"serial");
    if(sNo==NULL)
    continue;

    deviceName= udev_device_get_sysattr_value(parentdevice, "product");
    if(deviceName==NULL)
    continue;

    /*
    CHECKING FOR VALID NODE
    Finding the index of the node from the 11th position of device path
    and left shift it with 1,
    doing OR operation with refIndex() and assign it to refIndex
    */
    if(checkForValidNode(dev_path)==PASS)
    {
      count++;
    }
  }
  *numberOfDevices = count;
  udev_enumerate_unref(enumerate);  // FREE ENUMERATE
  udev_unref(udev);                 // FREE UDEV
  return PASS;
}

//DISPLAYS THE INFORMATION OF THE DEVICE WITH THE SPECIFIC INDEX
int getDeviceInfo(int deviceIndex,char *serialNumber, char *device_Name, char *productId,char *vendorId, char *devicePath)
{
  struct udev *udev;
  struct udev_device *device,*parentdevice;
  struct udev_enumerate *enumerate;
  struct udev_list_entry *devices, *dev_list_entry;
  int count=0;
  /* creating udev object */
  udev = udev_new();
  if (!udev)
  {
    printf("\nCANNOT CREATE UDEV CONTEXT...\n");
    return FAIL;
  }

  /* creating enumerate object */
  enumerate = udev_enumerate_new(udev);
  if (!enumerate)
  {
    printf("\nCANNOT CREATE ENUMERATE CONTEXT...\n");
    return FAIL;
  }

  //SEARCHING V4L SUB_SYSTEM IN ENUMERATE
  udev_enumerate_add_match_subsystem(enumerate, "video4linux");

  //SEND ENUMERATE AS THE INPUT PARAMETER
  udev_enumerate_scan_devices(enumerate);

  //FILLING UP THE DEVICE LIST
  devices = udev_enumerate_get_list_entry(enumerate);

  //ITERATING ALL THE ENTITIES OF THE LIST
  udev_list_entry_foreach(dev_list_entry, devices)
  {
    const char *path=NULL,*sNo=NULL,*deviceName=NULL;
    const char *product_id, *vendor_id;

    path = udev_list_entry_get_name(dev_list_entry);

    //GETTING ALL THE DEVICES IN THE ABOVE PATH AND RETURN THE CHAR POINTER TO device
    device = udev_device_new_from_syspath(udev, path);//allocating a new udev device object
    //GETTING THE PATH OF DEVICE NODE AND SET IT TO DEV_PATH
    const char *dev_path = udev_device_get_devnode(device);

    //CHECKING FOR TYPE AND SUB SYSTEM IN THE DEVICE AND STORE THE POINTER TO PARENT DEVICE
    parentdevice = udev_device_get_parent_with_subsystem_devtype(device,"usb","usb_device");
    if(!parentdevice)
    continue;
    vendor_id =  udev_device_get_sysattr_value(parentdevice,"idVendor");

    product_id = udev_device_get_sysattr_value(parentdevice,"idProduct");

    sNo = udev_device_get_sysattr_value(parentdevice,"serial");
    if(sNo==NULL)
    continue;

    deviceName = udev_device_get_sysattr_value(parentdevice, "product");
    if(deviceName==NULL)
    continue;

    /*CHECKING FOR VALID NODE
    Finding the index of the node from the 11th position of device path
    and left shift it with 1,
    doing OR operation with refIndex() and assign it to refIndex
    */
    if(checkForValidNode(dev_path)==PASS)
    {
      // refIndex = refIndex | (1 << (int)(dev_path[10]-'0'));
      count++;
      if(count==deviceIndex)
      {
        strcpy(serialNumber,sNo);
        strcpy(device_Name,deviceName);
        strcpy(productId,product_id);
        strcpy(vendorId,vendor_id);
        strcpy(devicePath,dev_path);
        break;
      }
    }
  }
  udev_enumerate_unref(enumerate);  // FREE ENUMERATE
  udev_unref(udev);                 // FREE UDEV
  return PASS;
}

//GETTING THE CURRENT DEVICE FORMAT
int getCurrentFormat(int *frame_height,int *frame_width,char *pixelFormat)
{
  struct v4l2_format frmt;
  char pix_fmt[PIXEL_FORMAT];
  // int frame_height,frame_width,bytesPerLine;
  frmt.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;                      //SETTING THE FORMAT TYPE
  if(ioctl(fd,VIDIOC_G_FMT,&frmt)<0)                          //GETTING THE FORMAT
  {
    printf("\nFAILED IN GETTING THE FORMAT\n");
    return FAIL;
  }
  *frame_height = frmt.fmt.pix.height;
  *frame_width = frmt.fmt.pix.width;

  memset(pix_fmt, 0, PIXEL_FORMAT);
  pix_fmt[0] = (char)(frmt.fmt.pix.pixelformat & 0xff);
  pix_fmt[1] = (char)((frmt.fmt.pix.pixelformat >> 8) & 0xff);
  pix_fmt[2] = (char)((frmt.fmt.pix.pixelformat >> 16) & 0xff);
  pix_fmt[3] = (char)((frmt.fmt.pix.pixelformat >> 24) & 0xff);
  pix_fmt[4] = '\0';
  strcpy(pixelFormat,pix_fmt);
  return PASS;
}

//GIVES THE NUMBER OF FORMATS SUPPORTED BY THE DEVICE
int getFormats(int *formats)
{
  struct v4l2_fmtdesc formatDescriptor;
  struct v4l2_frmsizeenum frameSize;
  struct v4l2_frmivalenum frameIntervalValue;

  //INITIALIZING ALL THE VALUES TO 0 IN THE STRUCTURE
  memset(&formatDescriptor, 0, sizeof(formatDescriptor));
  memset(&frameSize, 0, sizeof(frameSize));
  memset(&frameIntervalValue, 0, sizeof(frameIntervalValue));

  formatDescriptor.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  formatDescriptor.index = 0;
  int count=0;
  char pix_fmt[PIXEL_FORMAT];
  if(fd < 0)
  {
    printf("\nFAILED IN OPENING THE DEVICE(GET FORMATS)\n");
    return FAIL;
  }

  /*
  ENUMERATING FORMAT DESCRIPTION FROM VIDIOC_ENUM_FMT
  ENUMERATING THE FRAME SIZES FOR THAT DESCRIPTION
  ENUMERATING FRAME INTERVAL VALUE FOR THAT PARTICULAR FRAMES
  */
  while(ioctl(fd,VIDIOC_ENUM_FMT,&formatDescriptor) == 0)
  {
    formatDescriptor.index++;

    frameSize.pixel_format = formatDescriptor.pixelformat;
    frameSize.index = 0;
    while((ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frameSize)) == 0)
    {
      if(frameSize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
      {
        frameSize.index++;
        frameIntervalValue.index = 0;
        frameIntervalValue.width = frameSize.discrete.width;
        frameIntervalValue.height = frameSize.discrete.height;
        frameIntervalValue.pixel_format=frameSize.pixel_format;
        while((ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frameIntervalValue)) == 0)
        {
          if(frameIntervalValue.type == V4L2_FRMIVAL_TYPE_DISCRETE)
          {
            frameIntervalValue.index++;
            count++;
          }
          else
          {
            printf("\nFRAME INTERVAL VALUE IS NOT V4L2_FRMIVAL_TYPE_DISCRETE TYPE\n ");
            return FAIL;
          }
        }
      }
      else
      {
        printf("\nFRAME SIZE IS NOT V4L2_FRMSIZE_TYPE_DISCRETE TYPE\n");
        return FAIL;
      }
    }
  }
  *formats=count;
  return PASS;
}

//GETTING THE FORMAT TYPES
int getFormatType(int formatIndex, char *formatType, int *width, int *height, int *fps)
{
  struct v4l2_fmtdesc formatDescriptor;
  struct v4l2_frmsizeenum frameSize;
  struct v4l2_frmivalenum frameIntervalValue;

  //INITIALIZING ALL THE VALUES TO 0 IN THE STRUCTURE
  memset(&formatDescriptor, 0, sizeof(formatDescriptor));
  memset(&frameSize, 0, sizeof(frameSize));
  memset(&frameIntervalValue, 0, sizeof(frameIntervalValue));

  //int *pixelformat;
  formatDescriptor.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  formatDescriptor.index = 0;
  int count=0;
  char pix_fmt[PIXEL_FORMAT];
  if(fd < 0)
  {
    printf("\nFAILED IN OPENING THE DEVICE(GET FORMATS TYPE)\n");
    return FAIL;
  }

  /*
  ENUMERATING FORMAT DESCRIPTION FROM VIDIOC_ENUM_FMT
  ENUMERATING THE FRAME SIZES FOR THAT DESCRIPTION
  ENUMERATING FRAME INTERVAL VALUE FOR THAT PARTICULAR FRAMES
  */

  while(ioctl(fd,VIDIOC_ENUM_FMT,&formatDescriptor) == 0)
  {
    formatDescriptor.index++;
    frameSize.pixel_format = formatDescriptor.pixelformat;
    frameSize.index = 0;
    while((ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frameSize)) == 0)
    {
      if(frameSize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
      {
        frameSize.index++;
        frameIntervalValue.index = 0;
        frameIntervalValue.width = frameSize.discrete.width;
        frameIntervalValue.height = frameSize.discrete.height;
        frameIntervalValue.pixel_format=frameSize.pixel_format;
        while((ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frameIntervalValue)) == 0)
        {
          if(frameIntervalValue.type == V4L2_FRMIVAL_TYPE_DISCRETE)
          {
            frameIntervalValue.index++;
            if(count == formatIndex)
            {
              memset(pix_fmt, 0, PIXEL_FORMAT);
              pix_fmt[0] = (char)(formatDescriptor.pixelformat & 0xff);
              pix_fmt[1] = (char)((formatDescriptor.pixelformat >> 8) & 0xff);
              pix_fmt[2] = (char)((formatDescriptor.pixelformat >> 16) & 0xff);
              pix_fmt[3] = (char)((formatDescriptor.pixelformat >> 24) & 0xff);
              pix_fmt[4] = '\0';
              strcpy(formatType,pix_fmt);
              *width      = frameIntervalValue.width;
              *height     = frameIntervalValue.height;
              *fps        = (int)frameIntervalValue.discrete.denominator / frameIntervalValue.discrete.numerator;
              return PASS;
            }
            count++;
          }
          else
          {
            printf("\nFRAME INTERVAL VALUE IS NOT V4L2_FRMIVAL_TYPE_DISCRETE TYPE\n ");
            return FAIL;
          }
        }
      }
      else
      {
        printf("\nFRAME SIZE IS NOT V4L2_FRMSIZE_TYPE_DISCRETE TYPE\n");
        return FAIL;
      }
    }
  }
  return PASS;
}

/*
Now after setting the format to the device, the next process is
to render the device and grab or capture the frame.

1.Querrying the capabilities of the device
2.Requesting for the buffer (Minimum 2 buffer is required for streaming)
3.Querrying the requested buffers' size and the physical address. By passing
those values in MMAP, used to get the space in the application.
4.Enqueuing the empty buffer. No buffer is enqueued before starting streaming,
driver returns an error as there is no buffer available.
So at least one buffer must be enqueued before starting streaming.
5.Starting video capture functionality (Stream On)
6.Start dequeuing
7.Grab the image
8.Stop video capture functionality (Stream Off)
*/

//1.CHECKING THE CAPABILITY OF THE DEVICE USING QUERYCAP
int queryCap()
{
  struct v4l2_capability cap;

  //QUERRYING THE CAPABILITIES OF THE DEVICE
  if(ioctl(fd,VIDIOC_QUERYCAP,&cap))
  {
    printf("\nQUERY CAP FAILED\n");
    return FAIL;
  }

  //CHECKING IF DEVICE SUPPORTS VIDEO CAPTURE
  if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
  {
    printf("\nDOES NOT SUPPORT VIDEO CAPTURE");
    return FAIL;
  }

  //CHECKING IF THE DEVICE SUPPORTS STREAMING
  if(!(cap.capabilities & V4L2_CAP_STREAMING))
  {
    printf("\nDOES NOT SUPPORT STREAMING\n");
    return FAIL;
  }
  return PASS;
}

//2.REQUEST FOR BUFFER
int requestBuffer()
{
  CLEAR(req);
  req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.count  = BUFFER_COUNT;
  req.memory = V4L2_MEMORY_MMAP;

  if(fd < 0)
  {
    printf("\nFAILED IN OPENING THE DEVICE(REQUEST QUERY)\n");
    return FAIL;
  }

  //VALIDATING THE BUFFER
  if(ioctl(fd, VIDIOC_REQBUFS, &req)<0)
  {
    printf("BUFFER REQUEST FAILED!!");
    return FAIL;
  }

  /*REQUEST COUNT LESS THAN MINIMUM VALUE THE BUFFER SIZE MUST NOT BE
  LESS THAN TWO AS WE CANNOT ABLE TO STREAM */
  if (req.count < 2)
  {
    printf("INSUFFICIENT BUFFER COUNT\n");
    return FAIL;
  }

  //ALLOCATING MEMORY FOR THE REQUESTED NUMBER OF BUFFERS
  buffers = (struct buffer*)calloc(req.count, sizeof (*buffers));
  if(!buffers)
  {
    printf("MEMORY NOT ALLOCATED!!\n");
    return FAIL;
  }
  return PASS;
}

//3.QUERRYING THE BUFFER
int querryBuffer()
{
  CLEAR(buf);
  if(fd < 0)
  {
    printf("\nFAILED IN OPENING THE DEVICE(QUERY BUFFER)\n");
    return FAIL;
  }
  for(int index=0;index<req.count;index++)
  {
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = index;

    //CHECKING IF QUERY BUFFER IS SUCCESSFUL OR NOT
    if(ioctl(fd, VIDIOC_QUERYBUF,&buf)<0)
    {
      printf("QUERY BUFFER FAILED!!\n");
      return FAIL;
    }

    buffers[index].length = buf.length;
    buffers[index].start=mmap(NULL,buf.length,PROT_READ|PROT_WRITE,MAP_SHARED,fd,buf.m.offset);
    //printf("querrybufferAFTER:%d\n",buf.length);

    if(buffers[index].start == MAP_FAILED) //VALIDATING FOR MMAP FAILURE
    {
      printf("MAPPING FAILED!\n");
      return FAIL;
    }
  }
  return PASS;
}

//UNMAPPING WHICH WAS MAPPED BEFORE
int unmap()
{
    for (int index = 0; index < req.count;index++)
    {
        //UNMAPPING THE DATA ACQUIRED FROM MMAP
        if(munmap(buffers[index].start, buffers[index].length)<0)
        {
            printf("MEMORY UNMAPPING FAILED\n");
            return FAIL;
        }
    }
    //Memory releasing
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.count = 0;
    req.memory =V4L2_MEMORY_MMAP;
    
    //RELEASING MEMORY BY REQUESTING BUFFER WITH COUNT 0
    if(ioctl(fd, VIDIOC_REQBUFS, &req)<0)
    {
        printf("MEMORY RELEASE FAILED\n");
        return FAIL;
    }
    return PASS;
}


//4.QUEUE BUFFER
int queueBuffer()
{
  if(fd < 0)
  {
    printf("\nFAILED IN OPENING THE DEVICE(QUEUE BUFFER)\n");
    return FAIL;
  }
  for(int index=0;index<req.count;index++)
  {
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = index;
    //VALIDATING QUEUE BUFFER
    if(xioctl(fd,VIDIOC_QBUF,&buf)<0)
    {
      printf("ENQUEUING FAILED\n");
      return FAIL;
    }
  }
  return PASS;
}

//5.SWITCHING ON THE STREAM
int streamOn()
{
  if(fd < 0)
  {
    printf("\nFAILED IN OPENING THE DEVICE(STREAM ON)\n");
    return FAIL;
  }
  //STREAMING ON THE DEVICE
  if(xioctl(fd,VIDIOC_STREAMON,&buf.type)<0)
  {
    printf("STREAMING IS FAILED!!\n");
    return FAIL;
  }
  return PASS;
}

//6.DEQUEUE BUFFER WHICH ENQUEUED IN QUEUE_BUFFER
int dequeueBuffer()
{
  CLEAR(buf);
  if(fd < 0)
  {
    printf("\nFAILED IN OPENING THE DEVICE(DEQUEUE BUFFER)\n");
    return FAIL;
  }
  int ioctlReturn;
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;

  //VALIDATING DEQUEUE BUFFER
  ioctlReturn = xioctl(fd,VIDIOC_DQBUF,&buf);

  if(fd<0)
  {
    printf("\nERROR IN OPENING FILE\n");
  }
  //printf("\ndequeuing..........\n");
  if(ioctlReturn < 0)
  {
    printf("DEQUEUE FAILED!\n");
    printf("SOMETHING WRONG:%s\n", strerror(errno));
    return FAIL;
  }

  //ALLOCATING MEMORY FOR TEMP_BUFFER
  temp_buffer = (unsigned char *)realloc((unsigned char*)temp_buffer,buf.bytesused);
  memcpy(temp_buffer,(unsigned char *)buffers[buf.index].start,buf.bytesused);

  bytesUsed = buf.bytesused; //ASSIGNING BYTESUSED IN THE VARIABLE

  //VALIDATING QUEUE BUFFER
  if(xioctl(fd, VIDIOC_QBUF,&buf)<0)
  {
    printf("QUEUE FAILED INSIDE DEQUEUE BUFFER!\n");
    return FAIL;
  }
  return PASS;
}

//THIS IS THE FUNCTION OF VOID POINTER TYPE FOR THREADING
void *render(void* arg)
{
  while(runningDequeue)
  {
    if(dequeueBuffer()!=PASS)
    {
      printf("\nDEQUEUE BUFFER FAILED\n");
      return (void*)FAIL;
    }
  }
  pthread_exit(NULL);
  //pthread_cancel(dequeueThread);
  pthread_mutex_destroy(&mutex);
}

//API TO START RENDERING
int startRender()
{
  if(requestBuffer()!=PASS)
  {
    printf("\nREQUEST BUFFER FAILED\n");
    return FAIL;
  }
  if(querryBuffer()!=PASS)
  {
    printf("\nQUERRY BUFFER FAILED\n");
    return FAIL;
  }
  if(queueBuffer()!=PASS)
  {
    printf("\nQUEUE BUFFER FAILED\n");
    return FAIL;
  }
  if(streamOn()!=PASS)
  {
    printf("\nSTREAM ON FAILED");
    return FAIL;
  }

  pthread_create(&dequeueThread,NULL,render,NULL);

  return PASS;
}

//7.API TO GRAB THE FRAME
unsigned char *grabFrame(int *bytesused)
{
  pthread_mutex_init(&mutex,NULL);
  pthread_mutex_lock(&mutex);
  unsigned char* data = NULL;

  //ASSIGNING THE BYTES USED WHICH IS DECLARED GLOBALY
  *bytesused=bytesUsed;
  data = (unsigned char*)malloc(*bytesused);
  memcpy(data,temp_buffer,*bytesused);

  if(data==NULL)
  {
    printf("\nMEMORY IS NOT COPIED TO DATA BUFFER FROM TEMP BUFFER\n");
  }
  pthread_mutex_unlock(&mutex);

  return data;
}

//OPENING THE DEVICE WITH THE SPECIFIED DEVICE INDEX
int openDevice(int deviceIndex)
{
  const char *product_id, *vendor_id;
  int count=0;
  struct udev *udev;
  struct udev_device *device,*parentdevice;
  struct udev_enumerate *enumerate;
  struct udev_list_entry *devices, *dev_list_entry;

  /* creating udev object */
  udev = udev_new();
  if (!udev)
  {
    printf("\nCANNOT CREATE UDEV CONTEXT\n");
    return FAIL;
  }

  /* creating enumerate object */
  enumerate = udev_enumerate_new(udev);
  if (!enumerate)
  {
    printf("CANNOT CREATE ENUMERATE CONTEXT\n");
    return FAIL;
  }
  udev_enumerate_add_match_subsystem(enumerate, "video4linux");
  udev_enumerate_scan_devices(enumerate);
  /* fillup device list */
  devices = udev_enumerate_get_list_entry(enumerate);
  if (!devices)
  {
    perror("FAILED TO GET DEVICE LIST...\n");
    return FAIL;
  }
  udev_list_entry_foreach(dev_list_entry, devices)
  {
    const char *path=NULL,*sNo=NULL;

    path = udev_list_entry_get_name(dev_list_entry);
    device = udev_device_new_from_syspath(udev, path);

    parentdevice = udev_device_get_parent_with_subsystem_devtype(device,"usb","usb_device");
    if(!parentdevice)
    return FAIL;

    const char *dev_path = udev_device_get_devnode(device);
    sNo = udev_device_get_sysattr_value(parentdevice,"serial");
    if(sNo==NULL)
    continue;
    vendor_id =  udev_device_get_sysattr_value(parentdevice,"idVendor");
    product_id = udev_device_get_sysattr_value(parentdevice,"idProduct");
    if(checkForValidNode(dev_path)==PASS)
    {
      // refIndex = refIndex | (1 << (int)(dev_path[10]-'0'));
      count++;
      if(count==deviceIndex)
      {
        fd=open(dev_path,O_RDWR|O_NONBLOCK);
        if(fd < 0)
        {
          printf("\nFAILED IN OPENING THE DEVICE\n");
          return FAIL;
        }
        break;
      }
    }
  }
  udev_enumerate_unref(enumerate); // free enumerate
  udev_unref(udev);                // free udev

  if(queryCap()!=PASS)
  {
    printf("\nQUERY CAPABILITY FAILED\n");
    return FAIL;
  }

  if(startRender()!=PASS)
  {
    printf("\nDEVICE FAILS TO RENDER\n");
    return FAIL;
  }
  return PASS;
}


//SETTING THE FORMAT IN THE PARTICULAR INDEX WHICH GOT FROM THE USER
int setFormat(int setFormatIndex)
{
  runningDequeue = false;
  //printf("start setformat\n");
  //pthread_exit(&dequeueThread);
  //printf("after set false...............\n");
  // pthread_cancel(dequeueThread);
  // pthread_mutex_destroy(&mutex);
  //printf("after mutex\n");
  if(streamOff()!=PASS)
  {
    printf("\nSTREAM OFF FAILED");
    return FAIL;
  }
  //  printf("after stream off\n");
  if(unmap()!=PASS)
  {
    printf("\nUNMAPPING IS FAILED");
    return FAIL;
  }
  //printf("after unmap\n");
  struct v4l2_fmtdesc formatDescriptor;
  struct v4l2_frmsizeenum frameSize;
  struct v4l2_frmivalenum frameIntervalValue;
  struct v4l2_format frmt;

  //INITIALIZING ALL THE VALUES TO 0 IN THE STRUCTURE
  memset(&formatDescriptor, 0, sizeof(formatDescriptor));
  memset(&frameSize, 0, sizeof(frameSize));
  memset(&frameIntervalValue, 0, sizeof(frameIntervalValue));

  char *description;
  //int *pixelformat;
  formatDescriptor.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  formatDescriptor.index = 0;
  int index=0,framePerSecond=0,width,height;
  int formatIndex=1; //FOR INDEXING THE PIXEL FORMAT FOR THE RESPECTIVE DEVICE

  // ENUMERATING FORMAT DESCRIPTION FROM VIDIOC_ENUM_FMT
  // ENUMERATING THE FRAME SIZES FOR THAT DESCRIPTION
  // ENUMERATING FRAME INTERVAL VALUE FOR THAT PARTICULAR FRAMES

  while(ioctl(fd,VIDIOC_ENUM_FMT,&formatDescriptor) == 0)
  {
    //printf("\nindex:%d desc:%s ",formatDescriptor.index,formatDescriptor.description);
    formatDescriptor.index++;
    frameSize.pixel_format = formatDescriptor.pixelformat;
    frameSize.index = 0;
    while((ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frameSize)) == 0)
    {
      if(frameSize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
      {
        //printf("frmsize index:%d w:%d  h:%d ",frameSize.index,frameSize.discrete.width,frameSize.discrete.height);
        frameSize.index++;
        frameIntervalValue.index = 0;
        frameIntervalValue.pixel_format = frameSize.pixel_format;
        frameIntervalValue.width = frameSize.discrete.width;
        frameIntervalValue.height = frameSize.discrete.height;

        while((ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frameIntervalValue)) == 0)
        {
          if(frameIntervalValue.type == V4L2_FRMIVAL_TYPE_DISCRETE)
          {
            //printf("frminterval index:%d",frameIntervalValue.index);
            frameIntervalValue.index++;
            //printf("\nSET FORMAT INDEX:%d\n",formatIndex);
            if(setFormatIndex == formatIndex)
            {
              frmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
              frmt.fmt.pix.pixelformat=formatDescriptor.pixelformat;
              frmt.fmt.pix.width=frameSize.discrete.width;
              frmt.fmt.pix.height= frameSize.discrete.height;
            //  printf("\ninside validation..............................\n");

              if((ioctl(fd, VIDIOC_S_FMT, &frmt))==0)
              {
                printf("\nTHE FORMAT %d IS SET\n",setFormatIndex);
                struct v4l2_streamparm parm;
                //Setting the parm type
                parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if(ioctl(fd, VIDIOC_G_PARM, &parm)){
                  printf("Get parameter failed\n");
                  return FAIL;
                }
                //Checking the device for fps set capability
                if (!(parm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME))
                {
                  printf("V4L2_CAP_TIMEPERFRAME not supported\n" );
                  return FAIL;
                }

                //If the device has set(fps) capability...then set the fps
                parm.parm.capture.timeperframe = frameIntervalValue.discrete;
                if(ioctl(fd, VIDIOC_S_PARM, &parm)){
                  printf("SET FPS FAILED\n");
                  return FAIL;
                }
                return PASS;
              }
              else
              {
                printf("SET FORMAT IS FAILED \n");
              }
            }
            formatIndex++;
          }
          else
          {
            printf("\nFRAME INTERVAL VALUE IS NOT V4L2_FRMIVAL_TYPE_DISCRETE TYPE\n ");
            return FAIL;
          }
        }
      }
      else
      {
        printf("\nFRAME SIZE IS NOT V4L2_FRMSIZE_TYPE_DISCRETE TYPE\n");
        return FAIL;
      }
    }
  }

  //START RENDERING AFTER THE FORMAT IS SET
  if(startRender()!=PASS)
  {
    printf("\nDEVICE FAILS TO RENDER\n");
    return FAIL;
  }
  return PASS;
}

//CLOSING THE DEVICE
int closeDevice()
{
  runningDequeue = false;
  pthread_exit((void*)dequeueThread);
  pthread_mutex_destroy(&mutex);
  if(streamOff()!=PASS)
  {
    printf("\nSTREAM OFF FAILED");
    return FAIL;
  }
  if(fd > 0)
  {
    close(fd);
  }
  return PASS;
}
