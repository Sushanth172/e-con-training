#include "v4l2dev.h"
#include <stdio.h>
#include <libudev.h>
#include <libusb-1.0/libusb.h>
#include <libv4l2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include <linux/uvcvideo.h>
#include <fcntl.h>
int refIndex=0;
int fd=-1;
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
    //  printf("\nTHIS IS INVALID NODE\n");
      return 0;
   }
    // printf("\nTHIS IS VALID NODE\n");
   return 1;
}


/*
int enumFormat()
{
  struct v4l2_format formatDescriptor;
  formatDescriptor.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  while (ioctl(fd,VIDIOC_ENUM_FMT,&formatDescriptor) == 0)
  {
    printf("PIXEL FORMAT:",formatDescriptor.pixelformat);
    printf("DESCRIPTION: ",formatDescriptor.description);
  }
}

*/
int getDeviceFormat()
{
    struct v4l2_format frmt;
    int frame_height,frame_width,bytesPerLine;
    frmt.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;                      //setting format type
    if(ioctl(fd,VIDIOC_G_FMT,&frmt)<0)                          //getting format
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

    return 0;

}

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
            break;
          }

        }
    }
    udev_enumerate_unref(enumerate); // free enumerate
    udev_unref(udev);                // free udev
    return fd;
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
        return;
    }

    /* creating enumerate object */
    enumerate = udev_enumerate_new(udev);
    if (!enumerate)
    {
        printf("\nCANNOT CREATE ENUMERATE CONTEXT...\n");
        return;
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
    return 0;
}


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
        return;
    }

    /* creating enumerate object */
    enumerate = udev_enumerate_new(udev);
    if (!enumerate)
    {
        printf("\nCannot create enumerate context.\n");
        return;
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
        vendor_id =  udev_device_get_sysattr_value(parentdevice,"idVendor");

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
