/*
TO GET THE NUMBER OF DEVICES
**************************************************************************************************************************
PARAMETER
numberOfDevices -> Passed as a reference from the sample application and this API will assign the number of
                   devices connected by checking its valid node
**************************************************************************************************************************
*/
__attribute__ ((visibility ("default"))) int getDeviceCount(int *numberOfDevices);



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
__attribute__ ((visibility ("default"))) int getDeviceInfo(int deviceIndex,char *serialNumber, char *device_Name, char *productId,char *vendorId, char *devicePath);


/*
TO OPEN THE DEVICE WITH THE INDEX FROM USER
**************************************************************************************************************************
INPUT PARAMETER
deviceIndex -> It is read from the sampleApplication and passed as a reference to openDevice,
               opens the device with the corresponding index
**************************************************************************************************************************
*/
__attribute__ ((visibility ("default"))) int openDevice(int deviceIndex);



/*
TO GET THE UVC CONTROLS FROM THE DEVICE
**************************************************************************************************************************
OUTPUT PARAMETERS
minimum       ->Minimum value for the particular controls
maximum       ->Minimum value for the particular controls
steppingDelta ->stepping value
currentValue  ->Current value for the device
default Value ->default value of particular controls for the device
**************************************************************************************************************************
*/
__attribute__ ((visibility ("default"))) int getUVCControls(int *minimum, int *maximum, int *steppingDelta, int *currentValue, int *defaultValue);



/*
TO SET THE UVC CONTROL BY GETTING THE VALUE FROM APPLICATION
**************************************************************************************************************************
INPUT PARAMETER
value -> Value from the application to set the UVC conrols
**************************************************************************************************************************
*/
__attribute__ ((visibility ("default"))) int setUVCControls(int value);



/*
TO GET THE CURRENT FORMAT OF THE DEVICE OPENED
**************************************************************************************************************************
OUTPUT PARAMETERS
frame_height -> Height of the frame
frame_width  -> Width of the frame
pixelFormat  -> format in which the pixels are arranged Eg: UYVY, Greyscale, MJPEG etc..
**************************************************************************************************************************
*/
__attribute__ ((visibility ("default"))) int getCurrentFormat(int *frame_height,int *frame_width,char *pixelFormat);



/*
TO GET THE NUMBER OF FORMATS IN THE DEVICE
**************************************************************************************************************************
OUTPUT PARAMETER
formats -> gets the number of formats supported by the device
**************************************************************************************************************************
*/
__attribute__ ((visibility ("default"))) int getFormats(int *formats);




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
__attribute__ ((visibility ("default"))) int getFormatType(int format, char *formatType, int *width, int *height, int *fps);



/*
TO SET THE FORMAT TO THE DEVICE
**************************************************************************************************************************
INPUT PARAMETER
setFormatIndex -> Index read from the sample application

From the specified index, the respective format is set to the device
**************************************************************************************************************************
*/
__attribute__ ((visibility ("default"))) int setFormat(int setFormatIndex);


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
__attribute__ ((visibility ("default"))) unsigned char *grabFrame(int *bytesused);



//TO CLOSE THE DEVICE
__attribute__ ((visibility ("default"))) int closeDevice();
