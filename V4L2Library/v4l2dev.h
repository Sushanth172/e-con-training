int getDeviceCount();
int getDeviceInfo(char *serialNumber, char *deviceName, char *productId,char *vendorId, char *devicePath, int deviceIndex);
int openDevice(int deviceIndex);
int getDeviceFormat();
int enumFormat();
int closeDevice();
int setFormat(int formatIndexFromUser);