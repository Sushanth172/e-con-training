#include "v4l2dev.h"
#include <stdio.h>
#include <stdlib.h>
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
int refIndex=0;
int fd=-1;

#define fun m##a##i##n
int fun()
{
}
//CHECKING FOR VALID NODE USING THE DEVICE PATH
int checkForValidNode(const char *dev_path)
{
  int cam_fd;
  struct v4l2_capability cam_cap;                                 //For getting capability of that device
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
  if (cam_cap.device_caps & V4L2_CAP_META_CAPTURE)                //If this Flag is enable that means invalid node
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
  if(fd > 0)
  {
    close(fd);
  }
  return PASS;
}

//DISPLAYS THE INFORMATION OF THE DEVICE WITH THE SPECIFIC INDEX
int getDeviceInfo(char *serialNumber, char *device_Name, char *productId,char *vendorId, char *devicePath, int deviceIndex)
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
