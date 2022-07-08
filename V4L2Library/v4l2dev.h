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
**************************************************************************************************************************
*/
int getFormatType(int format, char *formatType, int *width, int *height, int *fps);

/*
TO SET THE FORMAT TO THE DEVICE
**************************************************************************************************************************

**************************************************************************************************************************
*/
int setFormat(int setFormatIndex);

int startRender();

/*
TO GRAB THE FRAME
**************************************************************************************************************************

**************************************************************************************************************************
*/
// int grabFrame(unsigned char *data,int *bytesused);
unsigned char *grabFrame(int *bytesused);

//TO CLOSE THE DEVICE
int closeDevice();
