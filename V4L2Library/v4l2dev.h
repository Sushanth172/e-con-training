/*
TO GET THE NUMBER OF DEVICES
**************************************************************************************************************************
PARAMETER
numberOfDevices -> Passed as a reference from the sample application and this API will assign the number of
                   devices connected by checking its valid node
**************************************************************************************************************************
*/
int getDeviceCount(int *numberOfDevices);



/*
TO GET THE CONNECTED DEVICE' DATA
**************************************************************************************************************************
getDeviceInfo Parameters
INPUT PARAMETER
deviceIndex -> input parameter which gives index from main
               to the specified index, the corresponding device' data will be assigned as
               the output parameter
OUTPUT PARAMETER
serielNumber -> Seriel number of the device(unique to each device)
device_Name  -> Name of the device
productId    -> Id of the product(unique to each device)
vendorId     -> This Id is same for all the device as the vendor is same(econ systems)
devicePath   -> Path of the device
**************************************************************************************************************************
*/
int getDeviceInfo(int deviceIndex,char *serialNumber, char *device_Name, char *productId,char *vendorId, char *devicePath);



/*
TO OPEN THE DEVICE WITH THE INDEX FROM USER
**************************************************************************************************************************
INPUT PARAMETER
deviceIndex -> It is read from the sampleApplication and passed as a reference to openDevice,
               opens the device with the corresponding index
**************************************************************************************************************************
*/
int openDevice(int deviceIndex);



/*
TO GET THE CURRENT FORMAT OF THE DEVICE OPENED
**************************************************************************************************************************
OUTPUT PARAMETERS
frame_height -> Height of the frame
frame_width  -> Width of the frame
pixelFormat  -> format in which the pixels are arranged Eg: UYVY, Greyscale, MJPEG etc..
**************************************************************************************************************************
*/
int getCurrentFormat(int *frame_height,int *frame_width,char *pixelFormat);



/*
TO GET THE NUMBER OF FORMATS IN THE DEVICE
**************************************************************************************************************************
OUTPUT PARAMETER
formats -> gets the number of formats supported by the device
**************************************************************************************************************************
*/
int getFormats(int *formats);



/*
TO GET ALL THE FORMATS SUPPORTED BY THE DEVICE
**************************************************************************************************************************
INPUT PARAMETER
format -> gives the number of formats supported by the device
OUTPUT PARAMETERS
formatType -> Type of the format Eg: UYVY, Greyscale, MJPEG,etc...
width      -> Width of the frame
height     -> Height of the frame
fps        -> Frame Per Second
**************************************************************************************************************************
*/
int getFormatType(int format, char *formatType, int *width, int *height, int *fps);



/*
TO SET THE FORMAT TO THE DEVICE
**************************************************************************************************************************
INPUT PARAMETER
setFormatIndex -> Index read from the sample application

From the specified index, the respective format is set to the device
**************************************************************************************************************************
*/
int setFormat(int setFormatIndex);



/*
DEVICE RENDERING PROCESS
**************************************************************************************************************************
  querycap(),requestBuffer(),querryBuffer(),queueBuffer() are called in this API
Following this a thread is created in order to run dequeueBuffer() to make it
rendering infinitely.
  When the grabFrame() is called, at that particular time of allocated buffers
i.e temp_buffer in the dequeueBuffer() is passed into the memcpy and it is
copied to the another buffer(data).
  This buffer(data) is used to write a file in the sampleApplication.cpp
**************************************************************************************************************************
*/

/*
TO GRAB THE FRAME
**************************************************************************************************************************
OUTPUT PARAMETER
bytesused -> Bytes that are used in the dequeue buffer is assigned to this parameter
             which is passed as a reference

grabFrame() is locked with a locking mechanism called Mutex in order to protect it
from overriding
**************************************************************************************************************************
*/
unsigned char *grabFrame(int *bytesused);



//TO CLOSE THE DEVICE
int closeDevice();
