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
#define PASS 1
#define FAIL -1
#define PIXEL_FORMAT 5
#define BUFFER_COUNT 3
#define CLEAR(x) memset(&(x), 0, sizeof(x))

struct buffer{
  void   *start;
  size_t  length;
};
int bytesUsed=0;
int refIndex=0;
int fd=-1;
struct v4l2_buffer buf;
struct v4l2_requestbuffers req;
struct buffer *buffers = NULL;
char *temp_buffer = NULL;
struct v4l2_capability cam_cap;  //For getting capability of that device

#define fun m##a##i##n
int fun()
{
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
    // printf("\nTHIS IS INVALID NODE\n");
    return FAIL;
  }
  // printf("\nTHIS IS VALID NODE\n");
  return PASS;
}
//ENUMERATION OF PIXEL FORMATS SUPPORTED BY THE DEVICE
int getFormats(int *formats)
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
          }//VIDIOC_ENUM_FRAMEINTERVALS if else
        }//while VIDIOC_ENUM_FRAMEINTERVALS
      }//VIDIOC_ENUM_FRAMESIZES if
      else
      {
        printf("\nFRAME SIZE IS NOT V4L2_FRMSIZE_TYPE_DISCRETE TYPE\n");
        return FAIL;
      }
      //  formatDescriptor.index++;

    }//enumFormat
  }
  return PASS;
}

//SETTING THE FORMAT IN THE PARTICULAR INDEX WHICH GOT FROM THE USER
int setFormat(int formatIndexFromUser)
{
  //  printf("Inside setformat\n");
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
            if(formatIndexFromUser == formatIndex)
            {
              frmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
              frmt.fmt.pix.pixelformat=formatDescriptor.pixelformat;
              frmt.fmt.pix.width=frameSize.discrete.width;
              frmt.fmt.pix.height= frameSize.discrete.height;
              if((ioctl(fd, VIDIOC_S_FMT, &frmt))==0)
              {
                printf("\nFORMAT SET\n");
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
  return PASS;
}

//CLOSING THE DEVICE
int closeDevice()
{
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
      //refIndex = refIndex | (1 << (int)(dev_path[10]-'0'));
      //printf("Index: %d\nCount:%d\n",refIndex,count);
      count++;
    }
  }
  *numberOfDevices = count;
  udev_enumerate_unref(enumerate);  // FREE ENUMERATE
  udev_unref(udev);                 // FREE UDEV
  return PASS;
}
//DO IOCTL UNTILL THE RESOURCE IS AVAILABLE(RESOURCE TEMPORARILY UNAVAILABLE)
int xioctl(int fd,int request, void *arg)
{
    int resouceAvailableIndex;
    do {
        resouceAvailableIndex = ioctl(fd, request, arg);
    } while (resouceAvailableIndex == -1 && EAGAIN == errno);  //do ioctl until there is no EAGAIN error(resource temporarily unavailable, try again)

    return resouceAvailableIndex;
}

//CHECKING THE CAPABILITY OF THE DEVICE USING QUERYCAP
int queryCap()
{
  struct v4l2_capability cap;
  if(ioctl(fd,VIDIOC_QUERYCAP,&cap))                      //querrying capabilities
  {
    printf("\nQUERY CAP FAILED\n");
    return FAIL;
  }
  if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))            //checking if the device supports video capture
  {
    printf("\nDOES NOT SUPPORT VIDEO CAPTURE");
    return FAIL;
  }
  if(!(cap.capabilities & V4L2_CAP_STREAMING))                //checking if the device supports streaming
  {
    printf("\nDOES NOT SUPPORT STREAMING\n");
    return FAIL;
  }
  return PASS;
}
//REQUEST FOR BUFFER
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
  if(ioctl(fd, VIDIOC_REQBUFS, &req)<0)                               //checking if request buffer is success or not
  {
    printf("BUFFER REQUEST FAILED!!");
    return FAIL;
  }

  if (req.count < 2)                                                  //request count less than minimum value
  {
    printf("INSUFFICIENT BUFFER COUNT\n");
    return FAIL;
  }
  buffers = (struct buffer*)calloc(req.count, sizeof (*buffers));     //allocating memory for requsted buffer count
  if(!buffers)
  {
    printf("MEMORY NOT ALLOCATED!!\n");
    return FAIL;
  }
  return PASS;
}

//QUERRYING THE BUFFER
int querryBuffer()
{
  CLEAR(buf);
  if(fd < 0)
  {
    printf("\nFAILED IN OPENING THE DEVICE(QUERY BUFFER)\n");
    return FAIL;
  }
  //printf("querrybufferBEFORE:%d\n",buf.length);
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

//QUEUE BUFFER
int queueBuffer()
{
  if(fd < 0)
  {
    printf("\nFAILED IN OPENING THE DEVICE(QUEUE BUFFER)\n");
    return FAIL;
  }
  //printf("BEFORE:%d\n",buf.bytesused);
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
  // printf("\nAFTER:%d\n",buf.bytesused);
  // printf("\nTYPE:%d\n",buf.type);
  // printf("\nFLAGS:%d\n",buf.flags);
  // printf("\nSEQUENCE:%d\n",buf.sequence);
  // printf("\nFIELD:%d\n",buf.field);
  return PASS;
}

//SWITCHING ON THE STREAM
int streamOn()
{
  if(fd < 0)
  {
    printf("\nFAILED IN OPENING THE DEVICE(STREAM ON)\n");
    return FAIL;
  }
  if(xioctl(fd,VIDIOC_STREAMON,&buf.type)<0)                       //streaming on the device
  {
    printf("STREAMING IS FAILED!!\n");
    return FAIL;
  }
  return PASS;
}

//DEQUEUE BUFFER WHICH ENQUEUED IN QUEUE_BUFFER
int dequeueBuffer()
{
  CLEAR(buf);
  if(fd < 0)
  {
    printf("\nFAILED IN OPENING THE DEVICE(DEQUEUE BUFFER)\n");
    return FAIL;
  }
  int dequeueIndex=0;
  int ioctlReturn;
  //printf("\nbefore While\n");
  while(dequeueIndex < 12)
  {
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    //printf("before\n");
    //VALIDATING DEQUEUE BUFFER
    ioctlReturn = xioctl(fd,VIDIOC_DQBUF,&buf);
    //printf("IOCTL RETURNS %d\n",ioctlReturn);
    if(ioctlReturn < 0)
    {
      printf("SOMETHING WENT WRONG:%s\n", strerror(errno));
      printf("DEQUEUE FAILED!\n");
      return FAIL;
    }
    //ALLOCATING MEMORY FOR TEMP_BUFFER
    temp_buffer = (unsigned char *)realloc((unsigned char*)temp_buffer,buf.bytesused);
    memcpy(temp_buffer,(unsigned char *)buffers[buf.index].start,buf.bytesused);
    bytesUsed = buf.bytesused;
    //printf("BYTES USED...DEQUEUE:%d\n",bytesUsed);
    //VALIDATING QUEUE BUFFER
    if(xioctl(fd, VIDIOC_QBUF,&buf)<0)
    {
      printf("QUEUE FAILED INSIDE DEQUEUE BUFFER!\n");
      return FAIL;
    }
   dequeueIndex++;
  }
  return PASS;
}
//SWITCHING OFF THE STREAM
int streamOff()
{
  if(fd < 0)
  {
    printf("\nFAILED IN OPENING THE DEVICE(STREAM OFF)\n");
    return FAIL;
  }
  if(xioctl(fd, VIDIOC_STREAMOFF, &buf.type)<0)                    //streaming off the device
  {
    printf("STREAM OFF FAILED\n");
    return FAIL;
  }
  return PASS;
}


int startRender()
{
  if(queryCap()!=PASS)
  {
    printf("\nQUERY CAPABILITY FAILED\n");
    return FAIL;
  }
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
  // if(dequeueBuffer()!=PASS)
  // {
  //   printf("\nDEQUEUE BUFFER FAILED");
  //   return FAIL;
  // }
  

}

//GRAB FRAME API
int grabFrame(unsigned char *data,int *bytesused)
{
  startRender();
  //data=(unsigned char*)malloc(bytesUsed);
  *bytesused=bytesUsed;
  memcpy(data,temp_buffer,*bytesused);

  printf("\nFRAME GRABBED\n");
  return PASS;
}
