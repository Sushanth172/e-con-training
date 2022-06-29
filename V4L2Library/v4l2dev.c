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
       return 0;
   }

   //Check if the device is capable of streaming 
   if(ioctl(cam_fd, VIDIOC_QUERYCAP, &cam_cap) < 0)
   {
     printf("VIDIOC_QUERYCAP failure");
     close(cam_fd);
     return 0;
   }

   close(cam_fd);
   if (cam_cap.device_caps & V4L2_CAP_META_CAPTURE)                //If this Flag is enable that means invalid node
   {
     //printf("\nTHIS IS INVALID NODE\n");
      return 0;
   }
     // printf("\nTHIS IS VALID NODE\n");
   return 1;
}

//ENUMERATION OF PIXEL FORMATS SUPPORTED BY THE DEVICE
int enumFormat()
{
  struct v4l2_fmtdesc formatDescriptor;
  struct v4l2_frmsizeenum frameSize;
  struct v4l2_frmivalenum frameIntervalValue;

  //INITIALIZING ALL THE VALUES TO 0 IN THE STRUCTURE
  memset(&formatDescriptor, 0, sizeof(formatDescriptor));
  memset(&frameSize, 0, sizeof(frameSize));
  memset(&frameIntervalValue, 0, sizeof(frameIntervalValue));

  char *description;
  //int *pixelformat;
  formatDescriptor.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  int index=0,framePerSecond=0,frameInterval=0,width,height;
  int formatIndex=1; //FOR INDEXING THE PIXEL FORMAT FOR THE RESPECTIVE DEVICE

  /*
   ENUMERATING FORMAT DESCRIPTION FROM VIDIOC_ENUM_FMT
   ENUMERATING THE FRAME SIZES FOR THAT DESCRIPTION
   ENUMERATING FRAME INTERVAL VALUE FOR THAT PARTICULAR FRAMES
  */
  printf("\n---------------------------PIXEL FORMATS AVAILABLE IN THE DEVICE---------------------------------------");
  while(ioctl(fd,VIDIOC_ENUM_FMT,&formatDescriptor) == 0)
  {
    if(formatDescriptor.type == V4L2_BUF_TYPE_VIDEO_CAPTURE)
    {
     frameSize.pixel_format = formatDescriptor.pixelformat;
     frameSize.index = 0;
     while((ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frameSize)) == 0)
     {
      if(frameSize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
      {
        width = frameSize.discrete.width;
        height = frameSize.discrete.height;
        frameSize.index++;
        frameIntervalValue.index = 0;
        //frameIntervalValue.pixel_format = frameSize.pixel_format;
        frameIntervalValue.width = frameSize.discrete.width;
        frameIntervalValue.height = frameSize.discrete.height;
        frameIntervalValue.pixel_format=frameSize.pixel_format;
        while((ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frameIntervalValue)) == 0)
        {
          if(frameIntervalValue.type == V4L2_FRMIVAL_TYPE_DISCRETE)
          {
           frameInterval++;
           frameIntervalValue.index++;
           framePerSecond = (double)frameIntervalValue.discrete.denominator / frameIntervalValue.discrete.numerator;    
       // printf("\nFORMAT INDEX      :%d\nDESCRIPTION       :%s\nHEIGHT            :%d\nWIDTH             :%d\nFRAMES PER SECOND :%d\n\n",formatIndex,formatDescriptor.description,frameIntervalValue.height,frameIntervalValue.width,framePerSecond);
           printf("\n%d\tDESCRIPTION: %s\tHEIGHT: %d\tWIDTH: %d\tFRAMES PER SECOND : %d\t\n",formatIndex,formatDescriptor.description,frameIntervalValue.height,frameIntervalValue.width,framePerSecond);
           formatDescriptor.index++;  
           formatIndex++;                 
          }
          else
          {
           printf("\nFRAME INTERVAL VALUE IS NOT V4L2_FRMIVAL_TYPE_DISCRETE TYPE\n ");
           return -1;
          }
          printf("-------------------------------------------------------------------------------------------------------");
        }
      }
      else
      {
       printf("\nFRAME SIZE IS NOT V4L2_FRMSIZE_TYPE_DISCRETE TYPE\n ");
       return -1;
      }
     }
    }
  }
  return formatIndex;
}

//SETTING THE FORMAT IN THE PARTICULAR INDEX WHICH GOT FROM THE USER
int setFormat(int formatIndexFromUser)
{
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
  int index=0,framePerSecond=0,frameInterval=0,width,height;
  int formatIndex=1; //FOR INDEXING THE PIXEL FORMAT FOR THE RESPECTIVE DEVICE

  /*
   ENUMERATING FORMAT DESCRIPTION FROM VIDIOC_ENUM_FMT
   ENUMERATING THE FRAME SIZES FOR THAT DESCRIPTION
   ENUMERATING FRAME INTERVAL VALUE FOR THAT PARTICULAR FRAMES
  */
  while(ioctl(fd,VIDIOC_ENUM_FMT,&formatDescriptor) == 0)
  {
    if(formatDescriptor.type == V4L2_BUF_TYPE_VIDEO_CAPTURE)
    {
     frameSize.pixel_format = formatDescriptor.pixelformat;
     frameSize.index = 0;
     while((ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frameSize)) == 0)
     {
      if(frameSize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
      {
        width = frameSize.discrete.width;
        height = frameSize.discrete.height;
        frameSize.index++;
        frameIntervalValue.index = 0;
        //frameIntervalValue.pixel_format = frameSize.pixel_format;
        frameIntervalValue.width = frameSize.discrete.width;
        frameIntervalValue.height = frameSize.discrete.height;
        frameIntervalValue.pixel_format=frameSize.pixel_format;

        while((ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frameIntervalValue)) == 0)
        {
          if(frameIntervalValue.type == V4L2_FRMIVAL_TYPE_DISCRETE)
          {
           frameInterval++;
           frameIntervalValue.index++;
           framePerSecond = (double)frameIntervalValue.discrete.denominator / frameIntervalValue.discrete.numerator;   
           if(formatIndexFromUser==formatIndex)
           {

        
          
           }
         // printf("\n%d\tDESCRIPTION: %s\tHEIGHT: %d\tWIDTH: %d\tFRAMES PER SECOND : %d\t\n",formatIndex,formatDescriptor.description,frameIntervalValue.height,frameIntervalValue.width,framePerSecond);
           formatDescriptor.index++;  
           formatIndex++;                 
          }
          else
          {
           printf("\nFRAME INTERVAL VALUE IS NOT V4L2_FRMIVAL_TYPE_DISCRETE TYPE\n ");
           return -1;
          }
        }
      }
      else
      {
       printf("\nFRAME SIZE IS NOT V4L2_FRMSIZE_TYPE_DISCRETE TYPE\n ");
       return -1;
      }
     }
    }
  }
  return 1;
}
/*
   if(formatDescriptor.pixelformat == V4L2_PIX_FMT_GREY)
    {
	printf("PIXEL FORMAT :Y8 \n");
    }
    else if(formatDescriptor.pixelformat == V4L2_PIX_FMT_Y16)
    {
	printf("PIXEL FORMAT :Y16 \n");
    }

*/

//GETTING THE DEVICE FORMAT
int getDeviceFormat()
{
    struct v4l2_format frmt;
    int frame_height,frame_width,bytesPerLine;
    frmt.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;                      //SETTING THE FORMAT TYPE
    if(ioctl(fd,VIDIOC_G_FMT,&frmt)<0)                          //GETTING THE FORMAT
    {
        printf("\nFAILED IN GETTING THE FORMAT\n");
        return -1;
    }
    frame_height = frmt.fmt.pix.height;
    frame_width = frmt.fmt.pix.width;
    bytesPerLine = frmt.fmt.pix.bytesperline;

    printf("\nHEIGHT         :%d\n",frame_height);
    printf("WIDTH          :%d\n",frame_width);
    printf("BYTES_PER_LINE :%d\n",bytesPerLine);
    return 1;
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
        return -1;
    }

    /* creating enumerate object */
    enumerate = udev_enumerate_new(udev);
    if (!enumerate)
    {
        printf("CANNOT CREATE ENUMERATE CONTEXT\n");
        return -1;
    }
    udev_enumerate_add_match_subsystem(enumerate, "video4linux");
    udev_enumerate_scan_devices(enumerate);
    /* fillup device list */
    devices = udev_enumerate_get_list_entry(enumerate);
    if (!devices)
    {
        perror("FAILED TO GET DEVICE LIST...\n");
        return -1;
    }
    udev_list_entry_foreach(dev_list_entry, devices)
    {
        const char *path=NULL,*sNo=NULL;

        path = udev_list_entry_get_name(dev_list_entry);

        device = udev_device_new_from_syspath(udev, path);

        parentdevice = udev_device_get_parent_with_subsystem_devtype(device,"usb","usb_device");
        if(!parentdevice)
            return -1;

        const char *dev_path = udev_device_get_devnode(device);
        sNo = udev_device_get_sysattr_value(parentdevice,"serial");
        if(sNo==NULL)
            continue;

        vendor_id =  udev_device_get_sysattr_value(parentdevice,"idVendor");

        product_id = udev_device_get_sysattr_value(parentdevice,"idProduct");


        if(checkForValidNode(dev_path))
        {
          // refIndex = refIndex | (1 << (int)(dev_path[10]-'0'));
          count++;
          if(count==deviceIndex)
          {
            fd=open(dev_path,O_RDWR|O_NONBLOCK);
            if(fd < 0)
            {
             printf("\nFAILED IN OPENING THE DEVICE\n");
             return -1;
            }
            break;
          }

        }
    }
    udev_enumerate_unref(enumerate); // free enumerate
    udev_unref(udev);                // free udev
    return 1;
}
//CLOSING THE DEVICE
int closeDevice()
{
 if(fd > 0)
 {
  close(fd);
 }
  return 1;
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
        return 0;
    }

    /* creating enumerate object */
    enumerate = udev_enumerate_new(udev);
    if (!enumerate)
    {
        printf("\nCANNOT CREATE ENUMERATE CONTEXT...\n");
        return 0;
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
        if(checkForValidNode(dev_path))
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
    return 1;
}

//GETTING THE DEVICE COUNT 
int getDeviceCount()
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
        return 0;
    }

    /* creating enumerate object */
    enumerate = udev_enumerate_new(udev);
    if (!enumerate)
    {
        printf("\nCannot create enumerate context.\n");
        return 0;
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
 
        /*CHECKING FOR VALID NODE
          Finding the index of the node from the 11th position of device path
           and left shift it with 1, 
           doing OR operation with refIndex() and assign it to refIndex
        */

        if(checkForValidNode(dev_path))
        {
          //refIndex = refIndex | (1 << (int)(dev_path[10]-'0'));
          //printf("Index: %d\nCount:%d\n",refIndex,count); 
          count++;
         
        }
     }
    udev_enumerate_unref(enumerate);  // FREE ENUMERATE
    udev_unref(udev);                 // FREE UDEV
    return count;
}
