/* TO GET THE NUMBER OF DEVICES
**************************************************************************************************************************
ARGUMENTS
numberOfDevices ->
**************************************************************************************************************************
*/
int getDeviceCount(int *numberOfDevices);

//TO GET THE CONNECTED DEVICE' DATA
/*
**************************************************************************************************************************
getDeviceInfo Arguments
 deviceIndex ->

*/
int getDeviceInfo(int deviceIndex,char *serialNumber, char *device_Name, char *productId,char *vendorId, char *devicePath);

//TO OPEN THE DEVICE WITH THE INDEX FROM USER
int openDevice(int deviceIndex);

//TO GET THE CURRENT FORMAT OF THE DEVICE OPENED
int getCurrentFormat(int *frame_height,int *frame_width,char *pixelFormat);

//TO GET THE NUMBER OF FORMATS IN THE DEVICE
int getFormats(int *formats);

//TO GET ALL THE FORMATS SUPPORTED BY THE DEVICE
int getFormatType(int format, char *formatType, int *width, int *height, int *fps);

//TO SET THE FORMAT TO THE DEVICE
int setFormat(int formatIndexFromUser);

//TO GRAB THE FRAME
int grabFrame(unsigned char *data,int *bytesused);

//TO CLOSE THE DEVICE
int closeDevice();
