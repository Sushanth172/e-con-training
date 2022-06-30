int getDeviceCount(int *numberOfDevices);
int getDeviceInfo(char *serialNumber, char *deviceName, char *productId,char *vendorId, char *devicePath, int deviceIndex);
int openDevice(int deviceIndex);
int getCurrentFormat(int *frame_height,int *frame_width,int *bytesPerLine);
int getFormats(int *formats);
int getFormatType(int format, char *formatType, int *width, int *height, int *fps);
int closeDevice();
int setFormat(int formatIndexFromUser);
