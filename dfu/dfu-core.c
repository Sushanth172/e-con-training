#include "../cypress/cyusb_linux_1.0.4/include/cyusb.h"
#include "ecam-dfu.h"
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/videodev2.h>

#define BUFSIZE           2048
#define VENDORCMD_TIMEOUT	(5000)		// Timeout for each vendor command is set to 5 seconds.
#define GETHANDLE_TIMEOUT	(5)		// Timeout (in seconds) for getting a FX3 flash programmer handle.
#define MAX_FWIMG_SIZE	(512 * 1024)		// Maximum size of the firmware binary.
#define SPI_PAGE_SIZE	(256)			// Page size for SPI flash memory.
#define SPI_SECTOR_SIZE	(64 * 1024)		// Sector size for SPI flash memory.
#define FLASHPROG_VID	(0x2560)		// USB VID for the FX3 flash programmer.
#define MAX_WRITE_SIZE	(2 * 1024)		// Max. size of data that can be written through one vendor command.

// Round n up to a multiple of v.
#define ROUND_UP(n,v)	((((n) + ((v) - 1)) / (v)) * (v))

#define GET_LSW(v)	((unsigned short)((v) & 0xFFFF))
#define GET_MSW(v)	((unsigned short)((v) >> 16))
#define CLEAR_BUFFER(x) (memset(x,0,sizeof(x)))

static unsigned int cypressDevCount = 0;
uint32_t uiBootloaderFileSize = 0;
unsigned char *uBootLoaderBuf = NULL;

int num_devices_detected,crc=0;
cyusb_handle *h;

static int fwFilesize;

const int i2c_eeprom_size[] =
{
	1024,
	2048,
	4096,
	8192,
	16384,
	32768,
	65536,
	131072
};

char **busInfo = NULL;

int hyperyon_flag=FALSE,ecam51bUSB_flag = FALSE;
int fd=-1;
int retry;

void *temp_buffer=NULL;

/*@Name: gettickcount
 *@Description: function get Current time in ms
 *@Returns : time in milliseconds
 */
unsigned gettickcount()
{
	struct timeval tv;
	if(gettimeofday(&tv, NULL) != 0)
		return 0;

	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

/*@Name: usb_init
 *@Description: function initialize libusb
 *@Return: 0 on Success and LIBUSB_ERROR on faliure
 */
int usb_init()
{
	return libusb_init(NULL);
}

/*@Name: usb_close
 *@Description: This function releases all memory allocated and assing it to NULL
 *@Returns: Void
 */
void usb_close(libusb_device_handle *devHandle, libusb_device *devPtr)
{
	if(devHandle) {
		libusb_close(devHandle);
		devHandle = NULL;
	}

	if(fd > 0)	//Closing file descriptor
	{
		close(fd);
		fd = -1;
	}

#if 0
	if(devPtr) {
		free(devPtr);
		devPtr = NULL;
	}
#endif
	libusb_exit(NULL);
}

/*@Name:get_firmware_write_type
 *@Description: Function for checking firmware image type
 *@Returns: IMG_TYPE_1 or IMG_TYPE_2 or IMG_TYPE_3
 */
int get_firmware_write_type(char *image_path)
{
	FILE *fp;
	char temp[FCHECK_LEN+1];
	int retVal;

	fp = fopen(image_path, "rb");
	if(!fp) {
		printf(" fopen error \n");
		return FAILURE;
	}
	retVal = fread(temp, 1, FCHECK_LEN, fp);
	fclose(fp);
	if(retVal == FCHECK_LEN)
	{
		if ((temp[0] == 'C' && temp[1] == 'Y')) {
			return IMG_TYPE_3;
		}
		return temp[0];
	}
	return IMG_TYPE_1;
}

/*Name:check_file
 *Description: This function checks the image fie is present or not,,if pesent deduces the type of img based on first two bytes read.
 *Returns: Dfu imge type or FAILURE
 */
int check_file(char * image_path)
{
	FILE *fp;
	int retVal = 0;
	char temp[FCHECK_LEN+1];
	long filelength;
	char *fwbin;
	int firstfwlen, secondfwlen;

	fp = fopen(image_path, "rb");
	if(!fp) {
		printf(" fopen error \n");
		return FAILURE;
	}
/*
 * File Given May be of following types
 * 1. Old type firmware starting with "CY"
 * 2. New type of Firmware file
 * 	<1st byte Firmware update specific information>  = 0x01 - Write secondary firmware image
 * 							   0x02 - Write Both Primary image and secondary image
 * 	<econIMG> = ID information
 * 	<01>   = ID index
 * 	<0xAA,0xBB,0xCC,0xDD> = 4 byte firmware length information
 * 	..... = Bootloader Firmware
 * 	<econIMG> = ID information
 * 	<02>   = ID index
 * 	<0xAA,0xBB,0xCC,0xDD> = 4 byte firmware length information
 * 	..... = UVC Firmware
 */

	retVal = fread(temp, 1, FCHECK_LEN, fp);
	fclose(fp);
	if(retVal != FCHECK_LEN) {
		DEBUG_PRINT(("File Read Failed\r\n"));
		fclose(fp);
		return FAILURE;
	}else {
		if (temp[0] == 'C' && temp[1] == 'Y') {
			return DFU_INPUT_FIRMWARE_OLD_TYPE;
		}else {
			fp = fopen(image_path, "rb");
			if (fp) {
				fseek(fp, 0L, SEEK_END);
				filelength = ftell(fp);
				rewind(fp);
				fwbin = malloc(filelength);
				if (!fwbin) {
					return FAILURE;
				}
				fread(fwbin, 1, filelength, fp);
				memcpy(&firstfwlen, &fwbin[9], 4);
				memcpy(&secondfwlen, &fwbin[firstfwlen + 0x0d + strlen("econIMG") + 1], 4);
				if (	(fwbin[0] == 0x01 || fwbin[0] == 0x02)
					&& (strncmp(&fwbin[1],"econIMG", strlen("econIMG")) == 0)
					&& (fwbin[8] == 0x01)
					&& (strncmp(&fwbin[firstfwlen + 0x0d],"econIMG", strlen("econIMG")) == 0)
					&& (fwbin[firstfwlen + 0x0d + strlen("econIMG")] == 0x02)
					&& ((firstfwlen + secondfwlen + 1+7+1+4+7+1+4) == filelength)
					) {
					retVal = DFU_INPUT_FIRMWARE_NEW_TYPE;
				}else {
					retVal = DFU_INPUT_FIRMWARE_NOT_PRESENT;
				}
				free(fwbin);
				return retVal;
			}else {
				return FAILURE;
			}
		}
	}

	return SUCCESS;
}

// Added by Sankari: To find the number of devices which are in cypress mode
/*@Name: findNoOfcypressModeDevices
 *@Description: This function finds the number of devices which are in cypress mode
 *@Returns: Number of devices.
 */
int findNoOfcypressModeDevices(){
	int N;
	N = cyusb_open();
	return N;
}

/*@Name:get_fx3_prog_handle
 *@Description: Get the handle to the FX3 flash programmer device, if found.
 *@Returns: SUCCESS or FAILURE
 */
int get_fx3_prog_handle(void)
{
	printf("\n Trying to download flash programmer to RAM\n");

#if BINARY
	char img_path[PROD_LEN];
	memset(img_path,'\0',PROD_LEN);
	strcpy(img_path , getenv("APPDIR"));
	strcat(img_path,"/bootimg/bootloader.img");
	int r = fx3_usbboot_download(img_path);
#else
	int r = fx3_usbboot_download("../bootimg/bootloader.img");
#endif
	if ( r != 0 ) {
		printf("\n Failed to download flash prog utility\n");
		return -1;
	}
	return 0;
}

/*@Name: updateCypressdevlist
 *@Description: This function updates the cypress devices list
 *@Returns: SUCCESS or FAILURE
 */
int updateCypressdevlist()
{
	int i, r, num_interfaces, index = 0;
	char tbuf[60];
	struct libusb_config_descriptor *config_desc = NULL;

	for ( i = 0; i < num_devices_detected; ++i ) {
		h = cyusb_gethandle(i);
		sprintf(tbuf,"VID=%04x,PID=%04x,BusNum=%02x,Addr=%d",
				cyusb_getvendor(h), cyusb_getproduct(h),
				cyusb_get_busnumber(h), cyusb_get_devaddr(h));
		r = cyusb_get_active_config_descriptor (h, &config_desc);
		if ( r ) {
			return -1;
		}
		num_interfaces = config_desc->bNumInterfaces;
		while (num_interfaces){
			if (cyusb_kernel_driver_active (h, index)){
				cyusb_detach_kernel_driver (h, index);
			}
			index++;
			num_interfaces--;
		}
		cyusb_free_config_descriptor (config_desc);

		// Check if we have a handle to the FX3 flash programmer.
		r = get_fx3_prog_handle();
		if ( r != 0 ) {
			printf(" FX3 flash programmer not found\n");
			return -1;
		}
	}
}

/*@Name:find_econ_device
 *@Description: finds number of connected econ devices(both in imaging and dfu mode) in the system using pid.
 *@Returns: PRESENT , ABSENT or FALIURE
 */
int find_econ_device(device_id_s *device, uint8_t list, int index, int no_arg,
		libusb_device **devPtr, int econDevCount)
{
	libusb_device **devList = NULL;
	libusb_device_handle *h_handle = NULL;

	unsigned char string_Desc[PROD_LEN] = {0};
	size_t numUsbDevs = 0;
	ssize_t	idx = 0;
	uint8_t	econ_device = ABSENT;		// Flag to check the presence of e-con Systems device
	int retVal = 0, break_flag = 0;
	struct libusb_device_descriptor devDesc;
	int i;
	bool newDfuDeviceFound;

	numUsbDevs = libusb_get_device_list(NULL, &devList);

	if(index != NIL_INDEX && index != DFU_FLASH_MODE) {
		idx = (ssize_t)(index);
		break_flag = TRUE;
	} else{
		idx = 0;
	}

	while (idx < numUsbDevs) {
		*devPtr = devList[idx];

		//=====================================================================
		// Get the device descriptor for this device.
		//=====================================================================

		retVal = libusb_get_device_descriptor (*devPtr, &devDesc);
		if (retVal != LIBUSB_SUCCESS) {
			printf(" libusb_get_device_descriptor return %d\n", retVal);
			return retVal;
		}
		//==================================================================
		// Get and Check Product ID and Vendor ID
		//==================================================================

		// Print IDs of Camera boards from e-con System devices
		newDfuDeviceFound = false;
		DEBUG_PRINT(("devDesc.idVendor in %s:%x\n",  __func__, devDesc.idVendor));
		if((devDesc.idProduct == HYPERYON_PID)||(devDesc.idProduct == HYPERYON_DUAL_STREAM_PID))
			hyperyon_flag = TRUE;
		if(devDesc.idProduct == ECAM51B_USB_ECON_PID)
			ecam51bUSB_flag = TRUE;
		if(devDesc.idVendor == NXP_VID){
			econ_device = PRESENT;
			device->idVendor = devDesc.idVendor;
				device->idProduct = devDesc.idProduct;
				device->iProduct = devDesc.iProduct;
			return econ_device;
		}
		else if(devDesc.idVendor == ECON_VID)
		{
			DEBUG_PRINT(("econ vid\n"));
			if(index == DFU_FLASH_MODE)
			{
				char *selectedDevBusLocation = (char*)malloc(7*sizeof(char));
				sprintf(selectedDevBusLocation,"%d"",""%d", libusb_get_bus_number(*devPtr), libusb_get_device_address(*devPtr));
				DEBUG_PRINT(("selectedDevBusLocation:%s", selectedDevBusLocation));
				for(int j=0; j<econDevCount; j++)
				{
					DEBUG_PRINT(("busInfo[%d]:%s", j, busInfo[j]));
				}
				for(i=0; i<econDevCount; i++)
				{
					if(strcmp(selectedDevBusLocation, busInfo[i]) != 0)
					{
						continue;
					}else
					{
						break;
					}
				}
				if(i == econDevCount)
				{
					newDfuDeviceFound = true;
				}
				if((devDesc.idProduct == DFU_PID_DF00 || devDesc.idProduct == DFU_PID_DF01) && newDfuDeviceFound )
				{
					printf("\n Detected DFU device.. \n");
					econ_device = PRESENT;
					free(selectedDevBusLocation);

					break;
				}
				else
				{
					printf("\nEcon device absent\n");
					econ_device = ABSENT;
				}
				free(selectedDevBusLocation);
			}
			 else
				econ_device = PRESENT;
			if(list == TRUE) {
				retVal = libusb_open(*devPtr, &h_handle);
				if(retVal < 0) {
					printf(" Problem acquiring device handle in %s \n", __func__);
					return FAILURE;
				}
				retVal = libusb_get_string_descriptor_ascii(h_handle, devDesc.iProduct,
						string_Desc, PROD_LEN);
				printf (" IDX = %d, Product ID = %x, Product = %s \n",
						(int)idx, devDesc.idProduct, string_Desc);
				fflush(stdout);

				if(h_handle) {
					libusb_close(h_handle);
					h_handle = NULL;
				}
			}else {
				device->idVendor = devDesc.idVendor;
				device->idProduct = devDesc.idProduct;
				device->iProduct = devDesc.iProduct;
				if(index == DFU_FLASH_MODE) {
					idx++;
					continue;
				}
				 else
					break;
			}
		}
		else if(no_arg == FALSE) {
			econ_device = ABSENT;
			break;
		}

		if(list == TRUE || break_flag != TRUE)
			idx++;
		else
			break;
	}

	if(devList) {
		libusb_free_device_list(devList, 1); //free the list, unref the devices in it
		sleep(1);		// Sleep added to avoid a hang issue while freeing device_list
	}

	if(h_handle)
	{
		libusb_close(h_handle);
		h_handle = NULL;
	}

	DEBUG_PRINT(("%s exit ..\n", __func__));

	return econ_device;
}

/*@Name:reg_read_eeprom
 *@Description: to read the register containing the firmware version
 *@Returns: firmware register data or false
 */
unsigned char reg_read_eeprom(unsigned int reg_address)
{
	unsigned char data[]={0x00, 0x00, 0x00, 0x00};

  data[0] = EEPROM_DEVICE_ADDRESS_FOR_READ_OPERATION;
  data[1] = (reg_address & 0xFF);	// LSB of 16 bit EEPROM Register Address
  data[2] = (reg_address >> 8);	// MSB of 16 bit EEPROM Register Address
  data[3] = 0x00;

	if( !getControlValue(UVC_XU_HW_CONTROL_I2C_DEVICE, UVC_SET_CUR, 4, data))
	{
		return false;
	}

	data[0] = EEPROM_DEVICE_ADDRESS_FOR_READ_OPERATION;
  data[1] = (reg_address & 0xFF);	// LSB of 16 bit EEPROM Register Address
  data[2] = (reg_address >> 8);	// MSB of 16 bit EEPROM Register Address
  data[3] = 0x00;

	if(!getControlValue(UVC_XU_HW_CONTROL_I2C_DEVICE, UVC_GET_CUR, 4, data))
	{
		return false;
	}
	return data[3];
}

/*@Name:ecam51bUSB_readfirmware
 *@Description: to read the the firmware version
 *@Returns: true or false
 */
bool ecam51bUSB_readfirmware(version_s *version)
{
	int outputValues[6];
	if((outputValues[0] = reg_read_eeprom(EEPROM_51B_FWV_REG_ADDRESS1)) == FAILURE)
  {
     	perror("\n\n Error in reading EEPROM Firmware Version register1.");
      return false;
  }
 	if((outputValues[1] = reg_read_eeprom(EEPROM_51B_FWV_REG_ADDRESS2)) == FAILURE)
  {
		perror("\n\n Error in reading EEPROM Firmware Version register2.");
		return false;
  }
  if((outputValues[2] = reg_read_eeprom(EEPROM_FWV_REG_ADDRESS1)) == FAILURE)
  {
		perror("\n\n Error in reading EEPROM Firmware Version register3.");
		return false;
  }
  if((outputValues[3] = reg_read_eeprom(EEPROM_FWV_REG_ADDRESS2)) == FAILURE)
  {
		perror("\n\n Error in reading EEPROM Firmware Version register4.");
		return false;
  }
  if((outputValues[4] = reg_read_eeprom(EEPROM_FWV_REG_ADDRESS3)) == FAILURE)
  {
		perror("\n\n Error in reading EEPROM Firmware Version register5.");
		return false;
  }
  if((outputValues[5] = reg_read_eeprom(EEPROM_FWV_REG_ADDRESS4)) == FAILURE)
  {
		perror("\n\n Error in reading EEPROM Firmware Version register6.");
		return false;
  }
	version->MajorVersion  = outputValues[0];
	version->MinorVersion1 = outputValues[1];
	version->MinorVersion2 = (outputValues[2]<<8) + outputValues[3];	// SDK Version
	version->MinorVersion3 = (outputValues[4]<<8) + outputValues[5];	// SVN Version

	return true;
}

/*@Name:readfirmwareversion
 *@Description: reads firmware version of selected devices.
 *@Returns: SUCCESS or FAILURE
 */
int readfirmwareversion(device_id_s *device, version_s *version, unsigned char **strDesc,
		int index, int no_arg, libusb_device_handle **devHandle,
		libusb_device *devPtr)
{
	unsigned char *buffer = NULL;
	unsigned char *out_buffer = NULL;
	int transfer = 0;
	unsigned int start = 0, end = 0;
	uint8_t timeout = TRUE;
	int retVal = 0;
	int detach_interface = 0;
	if(hyperyon_flag) //Added by M.Vishnu Murali : If hyperyon is detected read fimwareversion via extension unit
	{
		if(open_xu("video4linux")==SUCCESS)
			retVal=hyperyon_read_firmware_version(version);
		return retVal;
	}
	else if(ecam51bUSB_flag)
	{
		if(open_xu("video4linux")==FAILURE)
		{
			printf("\nOpen extension unit failed");
			return FAILURE;
		}
		if(!ecam51bUSB_readfirmware(version))
			return FAILURE;
	  return SUCCESS;
	}
	retVal = claimdevice(*device, HID_INTERFACE, strDesc, &detach_interface, devHandle, devPtr);
	if(retVal == FAILURE)
		return retVal;

	if(!(*devHandle)) {
		printf(" devHandle == NULL \n");
		return FAILURE;
	}else
		DEBUG_PRINT(("%s devHandle != NULL \r\n", __func__));

	fflush(stdout);

	buffer = (unsigned char *) calloc(BUFFER_LENGTH + 1, sizeof(unsigned char));
	if(!buffer) {
		fprintf(stderr, " Out of memory, exiting\n");
		return -1;
	}

	buffer[0] = READFIRMWAREVERSION;

	retVal = libusb_interrupt_transfer(*devHandle, IN, buffer, BUFFER_LENGTH, &transfer, 100);
	if(retVal < 0) {
		printf("FUNC: %s -> libusb_interrupt_transfer WRITE return value = %d ", __func__, retVal);
		fflush(stdout);
		free(buffer);
		return retVal;
	}else {
		out_buffer = (unsigned char *) calloc(BUFFER_LENGTH + 1, sizeof(unsigned char));
		if(!out_buffer) {
			fprintf(stderr, " Out of memory, exiting\n");
			free(buffer);
			return -1;
		}

		/* Read the Firmware Version from the device */
		start = gettickcount();
		while(timeout) {
			/* Get a report from the device */
			retVal = libusb_interrupt_transfer(*devHandle, IN, out_buffer, BUFFER_LENGTH, &transfer,1000);
			if (retVal < 0) {
				printf("\n FUNC: %s -> libusb_interrupt_transfer READ return value = %d", __func__, retVal);
				free(buffer);
				free(out_buffer);
				return retVal;
			}else {
				if(out_buffer[0] == READFIRMWAREVERSION) {
					version->MajorVersion  = out_buffer[1];
					version->MinorVersion1 = out_buffer[2];
					version->MinorVersion2 = (out_buffer[3]<<8) + out_buffer[4];	// SDK Version
					version->MinorVersion3 = (out_buffer[5]<<8) + out_buffer[6];	// SVN Version
					timeout = FALSE;
				}
			}
			end = gettickcount();

			if((end - start) > TIMEOUT)
			{
				timeout = FALSE;
			}
		}
	}
	device_close(*devHandle, HID_INTERFACE, detach_interface);

	DEBUG_PRINT(("\n FUNC: %s -> device_close ", __func__));
	fflush(stdout);

	free(buffer);
	free(out_buffer);
	return SUCCESS;
}

/*@Name:ram_write
 *@Description: This function is used to write to FX3 RAM.
 *@Returns: 0- SUCCESS, -1=FALIURE
 */
static int ram_write(unsigned char *buf, unsigned int ramAddress, int len)
{
	int r;
	int index = 0;
	int size;

	while ( len > 0 ) {
		size = (len > MAX_WRITE_SIZE) ? MAX_WRITE_SIZE : len;
        r = cyusb_control_transfer(h, 0x40, 0xA0, GET_LSW(ramAddress), GET_MSW(ramAddress),
				&buf[index], size, VENDORCMD_TIMEOUT);
		if ( r != size ) {
			printf(" Vendor write to FX3 RAM failed\n");
			return -1;
		}

		ramAddress += size;
		index      += size;
		len        -= size;
	}

	return 0;
}

/*@Name: readBootloaderFile
 *@Description: This function checks the bootloader file is present or not.
 *@returns: 0-SUCCESS,-1 = fir read failed ,-3 =File not found
 */
int readBootloaderFile(const char *fileName, unsigned char *buf, unsigned int *bootloaderFileSize)
{
	uint32_t bytesRead;
	int uiBootloaderFileSize;

	FILE *pfirmwareFile;

	pfirmwareFile = fopen(fileName, "rb");
	if ( pfirmwareFile < 0 ) {
		printf(" File not found\n");
		return -3;
	}

	fseek(pfirmwareFile, 0L, SEEK_END);
	uiBootloaderFileSize = ftell(pfirmwareFile);
	*bootloaderFileSize = uiBootloaderFileSize;
	fseek(pfirmwareFile, 0L, SEEK_SET);

	bytesRead = fread(buf, 1, uiBootloaderFileSize, pfirmwareFile);
	if(bytesRead != uiBootloaderFileSize)
	{
		printf(" Reading firmware file failed....\r\n");
		fclose(pfirmwareFile);
		return -1;
	}
	fclose(pfirmwareFile);
	printf(" Reading firmware file is success\n");
	return 0;
}

void CYWB_BL_4_BYTES_COPY(unsigned int *output, unsigned char * input, unsigned int index)
{

    *output = input[index]; // lsb-1
    *output |= (int)(input[index + 1] << 8); //lbs2
    *output |= (int)(input[index + 2] << 16); //lbs2
    *output |= (int)(input[index + 3] << 24); //lbs2
}

/*@Name: fx3_usbboot_download.
 *@Description: This function reads the bootloader file and writes it to required device.
 *@Return: 0-SUCCESS ,-1 = FAILURE , -2 = Failed to read firmware file ,-3 = failed ram_write ,-4 = checksum error
 */
int fx3_usbboot_download(const char *filename)
{
	unsigned char *fwBuf;
	unsigned int  *data_p;
	unsigned int i, checksum;
	unsigned int address, length;
	int r, index;

	unsigned int fwImagePtr = 0;
	unsigned int sectionLength = 0;
	unsigned int sectionAddress = 0;
	unsigned int downloadAddress = 0;
	unsigned int computeCheckSum = 0;
	unsigned int programEntry = 0;
	unsigned char *downloadbuf;

	downloadbuf=(unsigned char *)malloc(BUFSIZE);
	if(downloadbuf==NULL)
	{
		printf(" Memory Aloocation FAiled..\r\n");
		return -1;
	}

	fwBuf = (unsigned char *)calloc (1, MAX_FWIMG_SIZE);
	if ( fwBuf == 0 ) {
		printf(" Failed to allocate buffer to store firmware binary\n");
		//sb->showMessage("Error: Failed to get memory for download\n", 5000);
		return -1;
	}

	unsigned int bootloaderFileSize;
	r = readBootloaderFile(filename, fwBuf, &bootloaderFileSize);
	if ( r != 0 ) {
		printf(" Failed to read firmware file %s\n", filename);
		//sb->showMessage("Error: Failed to read firmware binary\n", 5000);
		free(fwBuf);
		return -2;
	}

	if ((fwBuf[fwImagePtr] != 0x43) || (fwBuf[fwImagePtr + 1] != 0x59) )
	{
		// signature doesn't match
		printf(" Signature does not match\n");
		return false;
	}


	// Run through each section of code, and use vendor commands to download them to RAM.
	index    = 4;
	checksum = 0;
	while ( index < bootloaderFileSize ) {
		data_p  = (unsigned int *)(fwBuf + index);
		length  = data_p[0];
		address = data_p[1];
		if (length != 0) {
			for (i = 0; i < length; i++)
				checksum += data_p[2 + i];
			r = ram_write(fwBuf + index + 8, address, length * 4);
			if (r != 0) {
				printf(" Failed to download data to FX3 RAM\n");
				//sb->showMessage("Error: Write to FX3 RAM failed", 5000);
				free(fwBuf);
				return -3;
			}
		} else {
			if (checksum != data_p[2]) {
				printf (" Checksum error in firmware binary\n");
				//sb->showMessage("Error: Firmware checksum error", 5000);
				free(fwBuf);
				return -4;
			}

			r = cyusb_control_transfer(h, 0x40, 0xA0, GET_LSW(address), GET_MSW(address), NULL,
					0, VENDORCMD_TIMEOUT);
			if ( r != 0 )
				printf(" Ignored error in control transfer: %d\n", r);
			break;
		}

		index += (8 + length * 4);
	}

	free(fwBuf);

	printf("\n Loading bootloader file is succeeded.\n");
	return 0;
}

/*@Name: getNumberOfEconDevices
 *@Description: gets the number of connected econdevices.
 *Returns: total count of econ devices.
 */
int getNumberOfEconDevices()
{
	int retVal, r;
	int totalEconDev = 0;

	libusb_device **list = NULL;
	libusb_context *context = NULL;


	r = libusb_init(NULL);
	if (r) {
	      printf(" Error in initializing libusb library...\n");
	      return -2;
	}

	size_t count = libusb_get_device_list(context, &list);

	for (size_t idx = 0; idx < count; idx++) {
		libusb_device *device = list[idx];
		struct libusb_device_descriptor devDesc = {0};

		retVal = libusb_get_device_descriptor (device, &devDesc);
		if (retVal != LIBUSB_SUCCESS) {
			printf(" libusb_get_device_descriptor return %d\n", retVal);
			return FAILURE;
		}

		if(devDesc.idVendor == FLASHPROG_VID || devDesc.idVendor == NXP_VID){

			totalEconDev++;
		}

	}
	libusb_free_device_list(list, 1);
	libusb_exit(NULL);
	return totalEconDev;

}

/*@Name: freeDevBusDetails
 *@Description: Frees the memory allocated for bus infos.
 *Returns: void
 */
void freeDevBusDetails(int econDevCount){
	for (int i=0;i<econDevCount;i++)
	{
		if(busInfo[i]){
    		free(busInfo[i]);
		}
	}
	if(busInfo){
		free(busInfo);
	}
}

/*@Name:enumerate_all_econ_devices
 *@Description: Lists all connected econ devices.
 *Returns:SUCCESS or FAILURE
 */
int enumerate_all_econ_devices(int *usbDeviceIndexValues, int econDevCountValue)
{
	int retVal, r;
	int econDevCnt = 0, i=0;

	libusb_device **list = NULL;
	libusb_context *context = NULL;
	libusb_device_handle *h_handle = NULL;
	unsigned char productName_Desc[PROD_LEN] = {0};

	r = libusb_init(NULL);
	if (r) {
	      printf(" Error in initializing libusb library...\n");
	      return -2;
	}

	busInfo=(char **)malloc(econDevCountValue*sizeof(char *));
	for (int i=0;i<econDevCountValue;i++)
	{
    		busInfo[i]=(char*)malloc(7*sizeof(char));
	}

	size_t count = libusb_get_device_list(context, &list);

	for (size_t idx = 0; idx < count; idx++) {
		libusb_device *device = list[idx];
		struct libusb_device_descriptor devDesc = {0};

		retVal = libusb_get_device_descriptor (device, &devDesc);
		if (retVal != LIBUSB_SUCCESS) {
			printf(" libusb_get_device_descriptor return %d\n", retVal);
			return FAILURE;
		}

		retVal = libusb_open(device, &h_handle);
		if(retVal < 0) {
			printf(" Problem acquiring device handle in %s \n", __func__);
			return FAILURE;
		}
		libusb_get_string_descriptor_ascii(h_handle, devDesc.iProduct, productName_Desc, PROD_LEN);
		DEBUG_PRINT(("idx in enumeration:%d, bus no:%d, bus address:%d\n ", idx, libusb_get_bus_number(device), libusb_get_device_address(device)));


		if(devDesc.idVendor == FLASHPROG_VID){
			*(usbDeviceIndexValues + econDevCnt) = idx;

			sprintf(busInfo[i],"%d"",""%d", libusb_get_bus_number(device), libusb_get_device_address(device));

			DEBUG_PRINT(("busInfo[i]:%s\n", busInfo[i]));
			i++;
			econDevCnt++;
			if(devDesc.idProduct == DFU_PID_DF01 || devDesc.idProduct == DFU_PID_DF00){
				printf (" %d - e-conDFU[PID:0x%x] \n", econDevCnt,  devDesc.idProduct);
			}else{
				printf (" %d - %s \n", econDevCnt, productName_Desc);
			}
		}
		if(devDesc.idVendor == NXP_VID)
		{
			*(usbDeviceIndexValues + econDevCnt) = idx;

			sprintf(busInfo[i],"%d"",""%d", libusb_get_bus_number(device), libusb_get_device_address(device));

			i++;
			econDevCnt++;
			printf (" %d - NXP-Device[PID:0x%x] \n", econDevCnt, devDesc.idProduct);
		}
		if(h_handle) {
			libusb_close(h_handle);
			h_handle = NULL;
		}
	}
	libusb_free_device_list(list, 1);
	libusb_exit(NULL);
	return SUCCESS;
}

/*@Name: claimdevice
 *@Description: This function opens the libusb handle and claims the interface of selected devices.
 *@Returns: SUCCESS or FAILURE
 */
int claimdevice(device_id_s device, uint8_t interface_class, unsigned char **strDesc, int *detach_interface,
		libusb_device_handle **devHandle, libusb_device *devPtr)
{
	struct libusb_interface *inter;
	struct libusb_interface_descriptor *interdesc;
	struct libusb_config_descriptor *config;
	int retVal = 0;
	int iface = 0, altset = 0;
	unsigned char *string_desc = NULL;
	int ascii_retry = 3;
	if(!devPtr) {
		DEBUG_PRINT(("\n devPtr is NULL inside %s", __func__));
		return FAILURE;
	}

	retVal = libusb_open(devPtr, devHandle);
	if(retVal < 0) {
		printf(" Problem acquiring device handle. Are you root?? \n");
		if (retVal == -1) {
			printf(ANSI_TEXT_BOLD ANSI_COLOR_RED
				"UNABLE TO ACCESS USB PORT, PLEASE TRY WITH OTHER USB PORT\n"
				ANSI_COLOR_RESET);
		}
		return FAILURE;
	}

	if(!(*devHandle)) {
		DEBUG_PRINT(("devHandle is NULL inside %s\n", __func__));
		return FAILURE;
	}
	string_desc = (unsigned char *) calloc(BUFFER_LENGTH, sizeof(unsigned char));
	if(!string_desc) {
		fprintf(stderr, " Out of memory, exiting\n");
		return -1;
	}

	if(device.idProduct != DFU_PID_DF01 && device.idProduct != DFU_PID_DF00)
	{
		do {
			retVal = libusb_get_string_descriptor_ascii(*devHandle, device.iProduct, string_desc, PROD_LEN);
			if (retVal < 0) {
				printf(" %s : libusb_get_string_descriptor_ascii fails \n", __func__);
			}

			if(retVal == strlen(string_desc)) {
				break;
			}else {
				DEBUG_PRINT(("%s ==> libusb_get_string_descriptor_ascii fails ..\r\n", __func__));
				printf (" Product ID = %x, Product = %s \n", device.idProduct, string_desc);
				break;
			}
		} while (--ascii_retry);

		if (!ascii_retry) {
			return FAILURE;
		}

		DEBUG_PRINT(("%s strlen = %d ..\r\n", __func__, (int)strlen(string_desc)));

		if(*strDesc)
			memcpy(*strDesc, string_desc, strlen(string_desc));
	}

	retVal = libusb_get_active_config_descriptor(devPtr, &config);
	if(retVal < 0) {
		DEBUG_PRINT(("%s => libusb_get_active_config_descriptor FAILED. Return value = %d\n", __func__, retVal));
		if(string_desc)
			free(string_desc);

		return FAILURE;
	}

	for(iface = 0; iface < config->bNumInterfaces; iface++) {
		inter = (struct libusb_interface *)&config->interface[iface];

		for( altset = 0; altset < inter->num_altsetting; altset++) {
			interdesc = (struct libusb_interface_descriptor *)&inter->altsetting[altset];

			if(interdesc->bInterfaceClass == interface_class ) {
				if(interdesc->bInterfaceClass == DFU_INTERFACE ) {
					*detach_interface = iface;
					break;
				}
				else if( interdesc->bInterfaceNumber == 2 ) {
					*detach_interface = iface;
					break;
				}
			}
		}
	}


	if(interdesc->bInterfaceClass != DFU_INTERFACE)	{
		retVal = libusb_kernel_driver_active(*devHandle, *detach_interface);
		if(retVal == 1)	{
			retVal = libusb_detach_kernel_driver(*devHandle, *detach_interface);
			if( retVal < 0) {
				printf(" libusb_detach_kernel_driver failed retVal = %d\n", retVal);
				libusb_free_config_descriptor(config);
				if(string_desc)
					free(string_desc);
				return FAILURE;
			}
			else
				DEBUG_PRINT(("%s ==> libusb_detach_kernel_driver else case ..\r\n", __func__));
		}
		else
			DEBUG_PRINT(("%s ==> libusb_kernel_driver_active else case ..\r\n", __func__));
	}

	retVal = libusb_claim_interface(*devHandle, *detach_interface); //claim interface of device (detach_interface)
	if(retVal < 0) {
		printf(" Cannot Claim Interface ==> error value = %d", retVal);
		libusb_attach_kernel_driver(*devHandle, *detach_interface);
		libusb_free_config_descriptor(config);

		if(string_desc)
			free(string_desc);

		return FAILURE;
	}else
		DEBUG_PRINT(("%s ==> libusb_claim_interface else case ..\r\n", __func__));

	sleep(2);

	libusb_free_config_descriptor(config);

	if(string_desc)
		free(string_desc);

	return SUCCESS;
}

/*@Name: device_close
 *@Description: This function releases the interfaces and deattach the kernel interface
 *@Returns:SUCCESS or FAILURE
 */
int device_close(libusb_device_handle *devHandle, int dfu_status, int detach_interface)
{
	int retVal = 0;

	retVal = libusb_release_interface(devHandle, detach_interface); //release the claimed interface

	if(retVal != 0) {
		printf("\n Cannot Release Interface , retVal = %d\n", retVal);
		return retVal;
	}

	if(dfu_status != DFU_INTERFACE) {
		retVal = libusb_attach_kernel_driver(devHandle, detach_interface);
		if(retVal != 0)
		{
			printf("\n Cannot attach_kernel_driver , retVal = %d\n", retVal);
			return retVal;
		}
	}

	return SUCCESS;
}

/*@Name: updateCypressModeDevices
 *@Description: updates the number of devices in cypress mode and also loads bootloader img to the respective devices..
 *@Returns: SUCCESS or FAILURE
 */
int updateCypressModeDevices(){
	int retVal = 0;
	retVal = findNoOfcypressModeDevices();
	num_devices_detected = retVal;

	if(retVal > 0){
		printf("\n Cypress mode device found: %d \n", retVal);
	}else if(retVal == 0){
	//	printf("\nNo cypress mode device is available\n");
		return retVal;
	}else{
		printf("\n LIBUSB ERROR \n");
	}

	// get cypress device list and load bootloader
	retVal = updateCypressdevlist();
	cyusb_close();

	sleep (1);
	return retVal;
}

/*@Name: ecam51bUSB_write_data
 *@Description: writes data as per the given length from the output buffer
 *@Returns: true or false
 */
bool ecam51bUSB_write_data(unsigned int len,unsigned char *g_out_packet_buf)
{
	if(len<=0)
	{
		printf("\n Invalid length.");
		return false;
	}
	int retVal = write(fd, g_out_packet_buf, len);
	if (retVal < 0) {
		printf("\n Write failed\n");
		return false;
	}
	return true;
}

/*@Name: ecam51bUSB_read_data
 *@Description: reads data as per the given length into the input buffer
 *@Returns: true or false
 */
bool ecam51bUSB_read_data(unsigned int len,unsigned char *g_in_packet_buf)
{
	memset(g_in_packet_buf, 0, (size_t)BUFSIZE * sizeof(*g_in_packet_buf));
	if(len<=0)
	{
		printf("\n Invalid length.");
		return false;
	}
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

/* Wait up to 5 seconds. */
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	if(select(fd+1, &rfds, NULL, NULL, &tv) < 0){
		printf("\n Select failed\n");
		return false;
	}
	// sleep(5);
	int retVal = read(fd, g_in_packet_buf, len);					// Read data from camera
	if (retVal < 0) {
		// printf("\n Read failed\n");
		perror("\nRead failed\n");
		// return false;
	}
	return true;
}

/*@Name: ecam51bUSB_prepare_sdphostBuffer
 *@Description: preparing sdphost buffer
 *@Returns: SUCCESS or FAILURE
 */
int ecam51bUSB_prepare_sdphostBuffer(unsigned int command_code,unsigned int memory_address,unsigned char *g_out_packet_buf,unsigned char *g_in_packet_buf)
{
	memset(g_in_packet_buf, 0, (size_t)BUFSIZE * sizeof(*g_in_packet_buf));
	memset(g_out_packet_buf, 0, (size_t)BUFSIZE * sizeof(*g_out_packet_buf));
	g_out_packet_buf[0]= CMD_OUT;
	g_out_packet_buf[1]= (command_code >> 8) & 0xFF;
	g_out_packet_buf[2]= command_code & 0xFF;
	g_out_packet_buf[3]= (memory_address >> 24) & 0xFF;
	g_out_packet_buf[4]= (memory_address >> 16) & 0xFF;
	g_out_packet_buf[5]= (memory_address >> 8) & 0xFF;
	g_out_packet_buf[6]= memory_address & 0xFF;
	if(command_code == WRITE_FILE)
	{
		g_out_packet_buf[9]= (MAGIC_NUMBER >> 8) & 0xFF;
		g_out_packet_buf[10]= MAGIC_NUMBER & 0xFF;
	}
	return SUCCESS;
}

/*@Name: ecam51bUSB_prepare_blhostBuffer
 *@Description: preparing blhost buffer
 *@Returns: SUCCESS or FAILURE
 */
bool ecam51bUSB_prepare_blhostBuffer(unsigned int tag,int flag, int no_arg,uint32_t arg1,uint32_t arg2,uint32_t arg3)
{
	memset(&cmdpkt, 0, sizeof(cmdpkt));
	cmdpkt.rpt_header[0]= CMD_OUT;
	unsigned int byteCount=(4*sizeof(uint8_t)) + (no_arg * sizeof(uint32_t));
	cmdpkt.rpt_header[2]= byteCount & 0xFF;
	cmdpkt.rpt_header[3]= (byteCount >> 8) & 0xFF;

	cmdpkt.cmd_header[0]= tag;
	cmdpkt.cmd_header[1]= flag;
	cmdpkt.cmd_header[3]= no_arg;

	cmdpkt.arguments[0]=arg1;
	cmdpkt.arguments[1]=arg2;
	cmdpkt.arguments[2]=arg3;

	return true;
}

/*@Name: ecam51bUSB_writeBootloader
 *@Description: This function opens,reads the bootloader image file and write it to device.
 *@Returns: SUCCESS or FAILURE
 */
int ecam51bUSB_writeBootloader(unsigned char *g_in_packet_buf)
{
	FILE *firmwarefp=NULL;
	uint32_t filesize,totalframes;
	int64_t bytesleft = 0;

	int index1;
	unsigned char tempBuffer[PACKET_SIZE];
 	int64_t bytesread;

	firmwarefp = fopen("../bootimg/RT1060_Flashloader_nopadding.bin","rb");
	if(!firmwarefp)
	{
		printf("\nfopen error from FUNC: %s \n", __func__);
		return FAILURE;
	}

	if(fseek(firmwarefp,0,SEEK_END)<0)
		printf("\nfseek error from FUNC: %s \n", __func__);

	filesize = ftell(firmwarefp);
	fseek(firmwarefp,0,SEEK_SET);
	fflush(stdout);

	totalframes=(filesize / PACKET_SIZE);

	if((filesize%PACKET_SIZE))
		totalframes++;

	bytesleft = filesize;

	for(index1 = 0;index1 < totalframes; index1++)
	{
		CLEAR_BUFFER(tempBuffer);
		tempBuffer[0]=0x02;//pad 0x02 at beginning for indicating its out data packet
			     //make sure to read only 1025 bytes including the padded value.
		if(bytesleft > PACKET_SIZE-1)
		{
			bytesread = fread(tempBuffer+1, 1, PACKET_SIZE-1, firmwarefp);
			if(bytesread != PACKET_SIZE-1)
			{
				printf("\nError reading %d frame.",index1);
			}
		}
		else
		{
			bytesread = fread(tempBuffer+1, 1, bytesleft+1, firmwarefp);
			if(bytesread != bytesleft)
			{
				printf("\nError reading last frame.");
			}
		}
		bytesleft -= bytesread;

		if(write(fd,tempBuffer,PACKET_SIZE)<0)
		{
			printf("Write error from FUNC: %s \n", __func__);
		}
	}
		if(!ecam51bUSB_read_data(RESPONSE1_DATASIZE,g_in_packet_buf))
		{
			return FAILURE;
		}
		else
		{
			if(!ecam51bUSB_read_data(RESPONSE2_DATASIZE,g_in_packet_buf))
			{
				return FAILURE;
			}
			return SUCCESS;
		}
}

/*@Name: ecam51bUSB_writeImage
 *@Description: This function opens,reads the secondary bootloader image file and write it to device.
 *@Returns: true or false
 */
bool ecam51bUSB_writeImage(unsigned char *imagePath,unsigned char *g_in_packet_buf)
{
	FILE *firmwarefp;
	uint32_t filesize, totalframes;
	int64_t bytesleft = 0,bytesread;
	float percent = 0.00;
	int index1;
	unsigned char temp[ECAM51B_USB_PACKET_SIZE];

	firmwarefp = fopen(imagePath,"rb");
	if(!firmwarefp)
	{
		printf("fopen error from FUNC: %s \n", __func__);
		return false;
	}

	printf("\n\n Reading file.. \n");

	fseek(firmwarefp,0,SEEK_END);
	filesize = ftell(firmwarefp);
	printf("\n Image file size = %d bytes", filesize);
	fseek(firmwarefp,0,SEEK_SET);
	fflush(stdout);

	totalframes=(filesize / ECAM51B_USB_PACKET_SIZE);

	if((filesize%ECAM51B_USB_PACKET_SIZE))
		totalframes++;

	bytesleft = filesize;

	printf(ANSI_TEXT_BOLD);
	printf(ANSI_COLOR_RED "\n\n\t\t DO NOT UNPLUG THE DEVICE!" ANSI_COLOR_RESET "\n");

	for(index1 = 0;index1 < totalframes; index1++)
	{
		CLEAR_BUFFER(temp);
		if(bytesleft > ECAM51B_USB_PACKET_SIZE-4)
		{
			bytesread = fread(temp+4, 1, ECAM51B_USB_PACKET_SIZE-4, firmwarefp);
			if(bytesread != ECAM51B_USB_PACKET_SIZE-4)
			{
				printf("\nError reading %d frame.",index1);
			}
			temp[0] = 0x02;															//Padding 0x0200 for every frame in case of ecam51bUSB
			temp[1] = 0x00;
			temp[2] = (bytesread) & 0xFF;
			temp[3] = (bytesread >> 8 ) & 0xFF;
		}
		else
		{
			bytesread = fread(temp+4, 1, bytesleft, firmwarefp);
			if(bytesread != bytesleft)
			{
				printf("\n Error reading last frame.");
			}
			temp[0] = 0x02;																//Padding 0x0200 for every frame in case of ecam51bUSB
			temp[1] = 0x00;
			temp[2] = (bytesread) & 0xFF;
			temp[3] = ((bytesread) >> 8 ) & 0xFF;
		}

		bytesleft -= bytesread;
		if(write(fd,temp,bytesread+4)<0)
		{
			printf("\n\n Write error from FUNC: %s", __func__);
		}
	}
		if(!ecam51bUSB_read_data(ECAM51B_USB_PACKET_SIZE,g_in_packet_buf))
		{
			return false;
		}
		printf("\n Firmware Updated!!\n");
		return true;
}

/*@Name: send_hid_command
 *@Description: This function sends hid command, reads response and checks if response is valid
 *@Returns: true or false
 */
bool send_hid_command(void *outBuf,int len, unsigned int check_response,unsigned char *g_in_packet_buf)
{
	if(write(fd,(unsigned char*)outBuf,len)<0)
	{
		printf("\nWrite error from FUNC: %s", __func__);
		return false;
	}
	if(!ecam51bUSB_read_data(BLHOST_READ_DATASIZE,g_in_packet_buf))
	{
		return false;
	}
	if(g_in_packet_buf[4]!=check_response)
	{
		printf("\nGet response failed");
		return false;
	}
	return true;
}

/*
 *@name: ecam51bUSB_change_to_DFU
 *@Description: function to switch ecam51bUSB device to primary bootloader mode via extension unit
 *@Return: true or false
 */
bool ecam51bUSB_change_to_DFU(unsigned char *g_out_packet_buf)
{
	unsigned char data[4]={0xA0,0x30,0xff,0x00};

	if(!getControlValue(0x18, UVC_SET_CUR, 4, data))
			return false;
	return true;
}
/*
 *@name: getControlValue
 *@Description: function to get control values from device via extension unit
 *@Return: true or false
 */
bool getControlValue(__u8 controlId, __u8 queryType, uint numberOfValues, __u8 *outputValues)
{
	struct uvc_xu_control_query xquery;
  __u16 size = 0;

  memset(&xquery, 0, sizeof(xquery));
	xquery.query = UVC_GET_LEN;
	xquery.size = 2;
	xquery.selector = controlId;
  xquery.unit = 4;
	xquery.data = (__u8 *)&size;

	if(ioctl(fd,UVCIOC_CTRL_QUERY,&xquery)<0)
	{
		printf("\nUVCIOC_CTRL_QUERY ioctl1 failed in %s",__func__);
		return false;
	}

  memset(&xquery, 0, sizeof(xquery));
  xquery.query = queryType;
  xquery.size = size;
	xquery.selector = controlId;
	xquery.unit = 4;
	xquery.data = outputValues;
	if(ioctl(fd,UVCIOC_CTRL_QUERY,&xquery)<0)
	{
		perror("\nUVCIOC_CTRL_QUERY ioctl2 failed");
	 	// return false;
	}
	return true;
}

/*@Name: send_bl_host_command
 *@Description: This function prepares blhost buffers and sends the command for the device to change into imaging mode
 *@Returns: true or false
 */
bool send_bl_host_command(char *image_path,unsigned char *g_in_packet_buf,int imageFilesize)
{
	if(open_xu("hidraw") == FAILURE)
	{
		printf("\n Opening device extension unit failed\n");
		return false;
	}
	int flag=0;
	ecam51bUSB_prepare_blhostBuffer(CMD_TAG_GETPROPERTY,flag,2,1,0,0);
	if(!send_hid_command(&cmdpkt,16,0xa7,g_in_packet_buf))
	{
		printf("\nCMD_TAG_GETPROPERTY1-fail\n");
		return false;
	}

	ecam51bUSB_prepare_blhostBuffer(CMD_TAG_FILLMEMORY,flag,3,0x2000,0x04,0xc0000007);
	if(!send_hid_command(&cmdpkt,20,0xa0,g_in_packet_buf))
	{
		printf("\nCMD_TAG_FILLMEMORY1-fail" );
		return false;
	}
	ecam51bUSB_prepare_blhostBuffer(CMD_TAG_FILLMEMORY,flag,3,0x2004,0x04,0x00000000);
	if(!send_hid_command(&cmdpkt,20,0xa0,g_in_packet_buf))
	{
		printf("\nCMD_TAG_FILLMEMORY2-fail" );
		return false;
	}

	ecam51bUSB_prepare_blhostBuffer(CMD_TAG_CONFIGMEMORY,flag,2,0x0009,0x2000,0x00);
	if(!send_hid_command(&cmdpkt,20,0xa0,g_in_packet_buf))
	{
		printf("\nCMD_TAG_CONFIGMEMORY1-fail" );
		return false;
	}
	ecam51bUSB_prepare_blhostBuffer(CMD_TAG_FLASH_ERASE_REGION,flag,3,0x60000000,95460,0x00);
	if(!send_hid_command(&cmdpkt,20,0xa0,g_in_packet_buf))
	{
		printf("\nCMD_TAG_FLASH_ERASE_REGION-fail" );
		return false;
	}

	ecam51bUSB_prepare_blhostBuffer(CMD_TAG_FILLMEMORY,flag,3,0x3000,0x04,0xF000000F);
	if(!send_hid_command(&cmdpkt,20,0xa0,g_in_packet_buf))
	{
		printf("\nCMD_TAG_FILLMEMORY3-fail" );
		return false;
	}

	ecam51bUSB_prepare_blhostBuffer(CMD_TAG_CONFIGMEMORY,flag,2,0x0009,0x3000,0x00);
	if(!send_hid_command(&cmdpkt,20,0xa0,g_in_packet_buf))
	{
		printf("\nCMD_TAG_CONFIGMEMORY2-fail" );
		return false;
	}
	ecam51bUSB_prepare_blhostBuffer(CMD_TAG_GETPROPERTY,flag,2,0x0b,0,0);
	if(!send_hid_command(&cmdpkt,16,0xa7,g_in_packet_buf))
	{
		printf("\nCMD_TAG_GETPROPERTY2-fail" );
		return false;
	}

	flag=1;
	ecam51bUSB_prepare_blhostBuffer(CMD_TAG_WRITEMEMORY,flag,3,0x60001000,imageFilesize,0x00);
	if(!send_hid_command(&cmdpkt,20,0xa0,g_in_packet_buf))
	{
		printf("\nCMD_TAG_WRITEMEMORY-fail" );
		return false;
	}
	if(!ecam51bUSB_writeImage(image_path,g_in_packet_buf))
	{
		printf("\nWRITE IMAGE-fail" );
		return false;
	}

	flag=0;
	ecam51bUSB_prepare_blhostBuffer(CMD_TAG_SETPROPERTY,flag,2,0x8380,0,0);
	if(!send_hid_command(&cmdpkt,16,0xa0,g_in_packet_buf))
	{
		printf("\nCMD_TAG_SETPROPERTY1-fail" );
		return false;
	}

	ecam51bUSB_prepare_blhostBuffer(CMD_TAG_SETPROPERTY,flag,2,0x3204,0,0);
	if(!send_hid_command(&cmdpkt,16,0xa0,g_in_packet_buf))
	{
		printf("\nCMD_TAG_SETPROPERTY2-fail" );
		return false;
	}
	return true;
}

/*@Name: updatefirmware
 *@Description: This function updates the firmware of the selected device.
 *@Returns: SUCCESS or FAILURE
 */
int updatefirmware(device_id_s *device, char *image_path, int *detach_interface, int index, int no_arg,
		libusb_device_handle **devHandle, libusb_device **devPtr, int econDevCount,int imageFilesize)
{
	DFU_STATUS dfu_status;
	int retVal = 0;
	int change_to_dfu = 1;
	int cypressFallback = 0;

	unsigned char g_out_packet_buf[BUFSIZE];
	unsigned char g_in_packet_buf[BUFSIZE];

	retVal = find_econ_device(device, FALSE, index, no_arg, devPtr, econDevCount);

	if(retVal == ABSENT){
		return retVal;
	}
	else if(retVal != PRESENT && retVal != ABSENT){
		return retVal;
	}

	if((device->idProduct == HYPERYON_DUAL_STREAM_PID)||(device->idProduct == HYPERYON_PID)) //Added on 05/05/2020 : for sending cmd to hyperyon for switch to dfu mode
	{
		retVal=open_xu("video4linux");
		if(retVal == FAILURE)
		{
			printf("\n Opening device extension unit failed\n");
			return FAILURE;
		}
		retVal=switch_to_dfu_mode();
		if(retVal == SUCCESS)
			printf("\n Device switched to dfu mode.\n");
		else
		{
			printf("\n Failed switching to dfu mode.\n" );
			return FAILURE;
		}
		printf(" Trying to find e-con DFU Mode device.\n");
		do
		{
			retVal = find_econ_device(device, FALSE, NIL_INDEX, TRUE, devPtr, econDevCount);
			if (retVal != PRESENT)
				sleep(1);
		}while(retVal != PRESENT || device->idProduct != DFU_PID_DF01 );
	}
  if (device->idProduct == DFU_PID_DF00)
	{
		/*
		 *  Device already in DFU MODE
		 *  Now Updating UVC firmware only allowed
		 */
		device->idProduct = DFU_PID_DF00;
		device->iProduct = 0;

		retVal = claimdevice(*device, DFU_INTERFACE, NULL, detach_interface, devHandle, *devPtr);
		if(retVal == FAILURE) {
			printf("\n %s ==> Cannot claim DFU_Interface\n", __func__);
			return retVal;
		}
	}
	else if (device->idProduct == DFU_PID_DF01)
	{ // from cypress mode
		/*
		 *  Device already in DFU MODE
		 *  Now Updating UVC firmware only allowed
		 */
		device->idProduct = DFU_PID_DF01;
		device->iProduct = 0;

		retVal = claimdevice(*device, DFU_INTERFACE, NULL, detach_interface, devHandle, *devPtr);
		if(retVal == FAILURE)
		{
			printf(" %s ==> Cannot claim DFU_Interface\n", __func__);
			return retVal;
		}
		cypressFallback = 1;
	}
	else if(device->idProduct == ECAM51B_USB_DFU_PID)
	{
		if(!send_bl_host_command(image_path,g_in_packet_buf,imageFilesize))
		{
			printf("\n Sending blhost command failed.");
			return FAILURE;
		}
		return SUCCESS;
	}
	else if(device->idProduct == NXP_PID || device->idProduct == ECAM51B_USB_ECON_PID)
	{
		if(device->idProduct == ECAM51B_USB_ECON_PID)
		{
			/*Device is in imaging mode. Hence we have to change it to
			primary bootloader mode before updating firmware*/
			retVal=open_xu("video4linux");
			if(retVal == FAILURE)
			{
				printf("\n Opening device extension unit failed\n");
				return FAILURE;
			}
			if(ecam51bUSB_change_to_DFU(g_out_packet_buf))
				printf("\n Device switched to primary bootloader mode.\n");
			else
			{
				printf("\n Failed switching to primary bootloader mode.\n" );
				return FAILURE;
			}
			printf("\n Trying to find e-con primary bootloader Mode device.\n");
			retry = FIND_RETRY+3;											//To avoid infinite loop and since the device takes about seven seconds to get enumerated
			do
			{
				retVal = find_econ_device(device, FALSE, NIL_INDEX, TRUE, devPtr, econDevCount);
				if (retVal != PRESENT)
					sleep(1);
				--retry;
			}while((retry!=0) && (retVal != PRESENT || device->idProduct != NXP_PID));
		}
		/*Device is in primary bootloader mode. Write primary bootloader image.*/
		retVal = open_xu("hidraw");
		if(retVal == FAILURE)
		{
			printf("\n Opening device extension unit failed\n");
			return FAILURE;
		}

		ecam51bUSB_prepare_sdphostBuffer(ERROR_STATUS,0,g_out_packet_buf,g_in_packet_buf);
		if(!ecam51bUSB_write_data(WRITE_DATASIZE,g_out_packet_buf))
			return FAILURE;
		if(!ecam51bUSB_read_data(RESPONSE1_DATASIZE,g_in_packet_buf))
			return FAILURE;
		if(!ecam51bUSB_read_data(RESPONSE2_DATASIZE,g_in_packet_buf))
			return FAILURE;

		ecam51bUSB_prepare_sdphostBuffer(WRITE_FILE,MEMORY_ADDRESS,g_out_packet_buf,g_in_packet_buf);
		if(!ecam51bUSB_write_data(WRITE_DATASIZE,g_out_packet_buf))
			return FAILURE;
		printf("\n Changing to DFU Mode..");
		if(ecam51bUSB_writeBootloader(g_in_packet_buf)==FAILURE)
		{
			printf("\n Writing primary bootloader image failed");
			return FAILURE;
		}

		ecam51bUSB_prepare_sdphostBuffer(JUMP_ADDRESS,MEMORY_ADDRESS,g_out_packet_buf,g_in_packet_buf);
		if(!ecam51bUSB_write_data(WRITE_DATASIZE,g_out_packet_buf))
			return FAILURE;
		if(!ecam51bUSB_read_data(RESPONSE1_DATASIZE,g_in_packet_buf))
			return FAILURE;

		printf("\n\n Changed to DFU mode.");

		usb_close(NULL,NULL);

		sleep(2);

		/*Device is in DFU mode. Write secondary bootloader image.*/

		if(!send_bl_host_command(image_path,g_in_packet_buf,imageFilesize))
		{
			printf("\n Sending blhost command failed");
			return FAILURE;
		}
		return SUCCESS;
	}
	else
	{
		change_to_dfu = 0;
		// Now, retVal = PRESENT, so proceed..
		retVal = change_to_dfu_mode(*device, detach_interface, devHandle, *devPtr, image_path);
		if(retVal < 0)
		{
			DEBUG_PRINT(("change_to_dfumode failed return value = %d \n", retVal));
			return retVal;
		}
		else
		{
			fflush(stdout);
			sleep(8);
			if(device->idProduct == SEE3CAM160_PID)     //Since See3CAM160_fallsback into cypress mode and
				retry = FIND_RETRY+5;                    //takes about 10 iterations to get enumerated.
			else
				retry = 1;
			do
			{
				printf("\n Trying to find Cypress Mode device\n");

				retVal = updateCypressModeDevices();
				if (retVal <= 0){
					sleep(3);
				}
				--retry;
			}while((retry!=0)&&(retVal <= 0));
			if(retVal > 0)
			{
				sleep(3);
				cypressFallback = 1;
			}
			retry = FIND_RETRY;
			do
			{
				printf("\n Trying to find e-con DFU Mode device\n");

				retVal = find_econ_device(device, FALSE, DFU_FLASH_MODE, TRUE, devPtr, econDevCount);
				if (retVal != PRESENT){
					sleep(1);
				}
				--retry;
			}while((retry!=0)&&(retVal != PRESENT));
			if(retVal != PRESENT)
			{
				printf("\n---e-con devices not found: %d\n",retVal);
					return FAILURE;
			}

			if(cypressFallback)
				device->idProduct = DFU_PID_DF01;
			else
				device->idProduct = DFU_PID_DF00;

			device->iProduct = 0;

			retVal = claimdevice(*device, DFU_INTERFACE, NULL, detach_interface, devHandle, *devPtr);
			if(retVal == FAILURE)
			{
				printf(" %s ==> Cannot claim DFU_Interface\n", __func__);
				return retVal;
			}
		}
	}

	if(!change_to_dfu_idlestate(*devHandle))
	{
		printf("\n Func: %s change_to_dfu_idlestate failed .. \n", __func__);
		device_close(*devHandle, DFU_INTERFACE, *detach_interface);
		return FAILURE;
	}
	if (get_firmware_write_type(image_path) == IMG_TYPE_3)
	{
		if(!dodownload(image_path, *devHandle))
		{
			printf("\n Func: %s dodownload failed .. \n", __func__);
			device_close(*devHandle, DFU_INTERFACE, *detach_interface);
			return FAILURE;
		}
		sendzerolengthpkt(*devHandle);
		getstatus(&dfu_status, *devHandle);
	}
	else if (((get_firmware_write_type(image_path) == IMG_TYPE_1) || (get_firmware_write_type(image_path) == IMG_TYPE_2))  && cypressFallback)
	{ // if fallback to cypress, then write both primary and secondary
		if(!do_download_prifwimg(image_path, *devHandle)) {
			printf("\n Func: %s do_download_prifwimg failed .. \n", __func__);
			device_close(*devHandle, DFU_INTERFACE, *detach_interface);
			return FAILURE;
		}
		fflush(stdout);
		sleep(8);
		retry = FIND_RETRY;
		do
		{
			printf("\n Trying to find e-con DFU Mode device\n");
			retVal = find_econ_device(device, FALSE, NIL_INDEX, TRUE, devPtr, econDevCount);
			if (retVal != PRESENT)
				sleep(1);
			--retry;
		}while((retry!=0)&&(retVal != PRESENT));   //Edited by M.Vishnu Murali :In order to prevent infinite loop check for 5 times if not return fail.
		if(retVal != PRESENT)
			return FAILURE;

		device->idProduct = DFU_PID_DF01;
		device->iProduct = 0;

		retVal = claimdevice(*device, DFU_INTERFACE, NULL, detach_interface, devHandle, *devPtr);
		if(retVal == FAILURE)
		{
			printf("%s ==> Cannot claim DFU_Interface\n", __func__);
			return retVal;
		}
		if(!do_download_secfwimg(image_path, *devHandle))
		{
			printf("\n Func: %s do_download_secfwimg failed .. \n", __func__);
			device_close(*devHandle, DFU_INTERFACE, *detach_interface);
			return FAILURE;
		}

	}
	else if (get_firmware_write_type(image_path) == IMG_TYPE_1 && !cypressFallback)
	{
		if(!do_download_secfwimg(image_path, *devHandle))
		{
			printf("\n Func: %s do_download_secfwimg failed .. \n", __func__);
			device_close(*devHandle, DFU_INTERFACE, *detach_interface);
			return FAILURE;
		}
	}
	else if (get_firmware_write_type(image_path) == IMG_TYPE_2 && !cypressFallback)
	{
		if (!change_to_dfu)
		{
			printf("\n Changed to dfu mode - writing primary firmware\n");
			if(!do_download_prifwimg(image_path, *devHandle)) {
				printf(" \n Func: %s do_download_prifwimg failed .. \n", __func__);
				device_close(*devHandle, DFU_INTERFACE, *detach_interface);
				return FAILURE;
			}
			fflush(stdout);
			sleep(8);
			do {
				printf("\n Trying to find e-con DFU Mode device\n");
				retVal = find_econ_device(device, FALSE, NIL_INDEX, TRUE, devPtr, econDevCount);
				if (retVal != PRESENT)
					sleep(1);
			}while(retVal != PRESENT);
			device->idProduct = DFU_PID_DF01;
			device->iProduct = 0;

			retVal = claimdevice(*device, DFU_INTERFACE, NULL, detach_interface, devHandle, *devPtr);
			if(retVal == FAILURE)
			{
				printf("\n %s ==> Cannot claim DFU_Interface\n", __func__);
				return retVal;
			}
		}
		else
		{
			printf(ANSI_COLOR_RESET ANSI_COLOR_RED ANSI_TEXT_BOLD
				"ALREADY IN DFU MODE, WRITING DFU FIRMWARE"
				" LEADS TO FIRMWARE CORRUPTION\n" );
			printf(ANSI_COLOR_RESET);
		}
		printf("\n writing secondary firmware\n\n");
		if(!do_download_secfwimg(image_path, *devHandle))
		{
			printf("\n Func: %s do_download_secfwimg failed .. \n", __func__);
			device_close(*devHandle, DFU_INTERFACE, *detach_interface);
			return FAILURE;
		}
	}
	else if(get_firmware_write_type(image_path) == IMG_HYPERYON)//Added by M.Vishnu Murali:Incase for hyperyon image the first byte will be 0x57
	{
		if(!change_to_dfu_idlestate(*devHandle))
		{
				printf("\n Func: %s change_to_dfu_idlestate failed .. \n", __func__);
				device_close(*devHandle, DFU_INTERFACE, *detach_interface);
				return FAILURE;
		}
		if(!dodownload(image_path, *devHandle))
		{
				printf("\n Func: %s dodownload failed .. \n", __func__);
				device_close(*devHandle, DFU_INTERFACE, *detach_interface);
				return FAILURE;
		}
		sendzerolengthpkt(*devHandle);
	}
	device_close(*devHandle, DFU_INTERFACE, *detach_interface);
	return SUCCESS;
}

bool ecam51bUSB_reset_device()
{
	unsigned char data[4]={0xA0,0x32,0x04,0x00};

	if(!getControlValue(0x18, UVC_SET_CUR, 4, data))
			return false;
	return true;
}
struct dfu_if {
    // struct usb_dfu_func_descriptor func_dfu;
    uint16_t quirks;
    uint16_t busnum;
    uint16_t devnum;
    uint16_t vendor;
    uint16_t product;
    uint16_t bcdDevice;
    uint8_t configuration;
    uint8_t interface;
    uint8_t altsetting;
    uint8_t flags;
    uint8_t bMaxPacketSize0;
    char *alt_name;
    char *serial_name;
    libusb_device *dev;
    libusb_device_handle *dev_handle;
    struct dfu_if *next;
}new;

int resetDfuDevice(device_id_s *device, int *detach_interface,libusb_device_handle **devHandle, libusb_device **devPtr)
{
	/*In case of other e-con devices send 0x3204 command to reset the device*/
	unsigned char *buffer = NULL;
 	int transfer = 0,retVal;

	device->idProduct = DFU_PID_DF01;
	device->iProduct = 0;

    retVal = claimdevice(*device, DFU_INTERFACE, NULL, detach_interface, devHandle, *devPtr);
	if(retVal == FAILURE)
	{
		printf("\n %s ==> Cannot claim DFU_Interface\n", __func__);
	    return retVal;		
	}		

	if(!(devHandle)) {
		printf(" devHandle == NULL \n");
		return FAILURE;
	}

	buffer = (unsigned char *) calloc(BUFFER_LENGTH + 1, sizeof(unsigned char));
		if(!buffer) {
			fprintf(stderr, " Out Of Memory, Exiting....\n");
			return -1;
	}

	buffer[0] = 0x32;
	buffer[1] = 0x04;

    // int LIBUSB_CALL libusb_interrupt_transfer(libusb_device_handle *dev_handle,
	// unsigned char endpoint, unsigned char *data, int length,
	// int *actual_length, unsigned int timeout);


	retVal = libusb_interrupt_transfer(*devHandle, OUT, buffer, BUFFER_LENGTH, &transfer, 1000);
	// printf("\nTRANSFER: %d\n",transfer);

	// retVal = libusb_control_transfer(*devHandle,0x32,0x04,0,0,NULL,0,1000);
	printf("\nControl Return:%d\n",retVal);

	if(retVal < 0) {
		printf("\nFUNC: %s -> libusb_interrupt_transfer WRITE return value = %d\n ", __func__, retVal);
		fflush(stdout);
		free(buffer);
		return retVal;
	}

	// device_close(*devHandle, DFU_INTERFACE, detach_interface);

	free(buffer);
	return SUCCESS;
}


/*@Name: reset_device
 *@Description: This function resets the selected device.
 *@Returns: SUCCESS or FAILURE
 */
int reset_device(device_id_s *device, unsigned char **strDesc, int index, libusb_device_handle **devHandle, libusb_device *devPtr)
{
	if(ecam51bUSB_flag)
	{
		unsigned char g_out_packet_buf[BUFSIZE];
		if(open_xu("video4linux")==FAILURE)
		{
			printf("\nOpen extension unit failed\n");
			return FAILURE;
		}
		if(!ecam51bUSB_reset_device())
			return FAILURE;
	  return SUCCESS;
	}
	/*In case of other e-con devices send 0x3204 command to reset the device*/
	unsigned char *buffer = NULL;
 	int transfer = 0,retVal;
 	int detach_interface = 0;

	retVal = claimdevice(*device, HID_INTERFACE, strDesc, &detach_interface, devHandle, devPtr);
	if(retVal == FAILURE)
		return retVal;

	if(!(*devHandle)) {
		printf(" devHandle == NULL \n");
		return FAILURE;
	}

	buffer = (unsigned char *) calloc(BUFFER_LENGTH + 1, sizeof(unsigned char));
		if(!buffer) {
			fprintf(stderr, " Out of memory, exiting\n");
			return -1;
	}

	buffer[0] = 0x32;
	buffer[1] = 0x04;

	retVal = libusb_interrupt_transfer(*devHandle, OUT, buffer, BUFFER_LENGTH, &transfer, 100);
	printf("\nTRANSFER: %d\n",transfer);
	
	printf("\nControl Return:%d\n",retVal);
	if(retVal < 0) {
		printf("FUNC: %s -> libusb_interrupt_transfer WRITE return value = %d ", __func__, retVal);
		fflush(stdout);
		free(buffer);
		return retVal;
	}

	device_close(*devHandle, HID_INTERFACE, detach_interface);

	free(buffer);
	return SUCCESS;
}

/*@Name: change_to_dfu_mode
 *@Description: This function sends changes the device from imageing mode to dfu mode.
 *@Returns: SUCCESS or FAILURE
 */
int change_to_dfu_mode(device_id_s device, int *detach_interface, libusb_device_handle **devHandle,
				libusb_device *devPtr, char *image_path)
{
	unsigned char buffer[BUFFER_LENGTH] = {0};
	unsigned char *empty_buffer = NULL;
	int transferred = 0;
	int retVal = 0;

	retVal = claimdevice(device, HID_INTERFACE, &empty_buffer, detach_interface, devHandle, devPtr);
	if(retVal == FAILURE) {
		printf("\n %s ==> Cannot claim HID_Interface\n", __func__);
		return retVal;
	}

	buffer[0] = FIRMWAREUPDATE;
	buffer[1] = 0xff;
	retVal = libusb_interrupt_transfer(*devHandle, OUT, buffer, BUFFER_LENGTH, &transferred, 100);
	if(retVal < 0) {
		printf("\n %s : libusb_interrupt_transfer failed - %d\n", __func__, retVal);
	}else {
		printf("\n Device going into DFU mode..\n");
		fflush(stdout);
	}
	/*
	 * Expecting reply from Board 0x32
	 */
	memset(buffer, 0x00, sizeof(buffer));
	retVal = libusb_interrupt_transfer(*devHandle, IN, buffer, BUFFER_LENGTH, &transferred,1000);
	if (retVal < 0)
	{
		printf("\n Assuming Device is in old firmware mode\n");
	}
	else if (buffer[0] == 0x32)
	{
		printf("\n Device is Now in New firmware mode\n");
		buffer[0] = 0x32;
		buffer[1] = get_firmware_write_type(image_path);
		retVal = libusb_interrupt_transfer(*devHandle, OUT, buffer, BUFFER_LENGTH, &transferred, 100);
		if(retVal < 0)
		{
			printf("\n %s : libusb_interrupt_transfer failed - %d\n", __func__, retVal);
		}

	}
	else
	{
		printf("\n Got Values from Firmware, But not expected 0x32. So, Assuming it as a old firmware mode\n");
	}

	if(*devHandle)
	{
		libusb_close(*devHandle);
		*devHandle = NULL;
	}
	return SUCCESS;
}

/*@Name: change_to_dfu_idlestate
 *@Description: This function sends command to change the state of the device to idle .
 *@Returns: 0 or 1
 */
int change_to_dfu_idlestate(libusb_device_handle *devHandle)
{
	DFU_STATUS dfu_status;
	int err = 0;
	if (getstatus(&dfu_status, devHandle) == FAILURE)
	{
		printf("\n Func: %s getstatus failed .. \n", __func__);
		return FAILURE;
	}
	switch(dfu_status.bState) {
		case DFU_STATE_appIDLE:
			printf("\n Func: %s case DFU_STATE_appIDLE \n", __func__);
			sendcommand(USB_REQ_DFU_DETACH, devHandle);		// 0x00
			getstatus(&dfu_status, devHandle);
			if(dfu_status.bState != DFU_STATE_dfuIDLE)
				err = 1;
			break;
		case DFU_STATE_dfuERROR:
			printf("\n Func: %s case DFU_STATE_dfuERROR \n", __func__);
			sendcommand(USB_REQ_DFU_CLRSTATUS, devHandle);		// 0x04
			getstatus(&dfu_status, devHandle);
			if(dfu_status.bState != DFU_STATE_dfuIDLE)
				err = 1;
			break;
		case DFU_STATE_dfuDNLOAD_IDLE:
			printf("\n Func: %s case DFU_STATE_dfuDNLOAD_IDLE \n", __func__);
			sendcommand(USB_REQ_DFU_ABORT, devHandle);			// 0x06
			getstatus(&dfu_status, devHandle);
			if(dfu_status.bState != DFU_STATE_dfuIDLE)
				err = 1;
			break;
		case DFU_STATE_dfuMANIFEST_SYNC:
			printf("\n Func: %s case DFU_STATE_dfuMANIFEST_SYNC \n", __func__);
			getstatus(&dfu_status, devHandle);
			if(dfu_status.bState != DFU_STATE_dfuIDLE)
				err = 1;
			break;
	}

	return err?0:1;
}

/*@Name: getstatus
 *@Description: This function sends command to get the current status of the devices.
 *@Returns: SUCCESS or FAILURE
 */
int8_t getstatus(DFU_STATUS *dfu_status, libusb_device_handle *devHandle)
{
	int i = 0;
	unsigned char iobuffer[6] = {0};
	int retVal = 0;

	if(!devHandle)
		return FAILURE;

	retVal = libusb_control_transfer(devHandle,
			(LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR),
			USB_REQ_DFU_GETSTATUS, 0, 0, iobuffer, 6, 3000); // (0x80 | (0x02 << 5))
	//printf("libusb_control_transfer error: %d, %s\n", retVal, libusb_error_name(retVal));

	if(retVal < 0) {
		printf("\n libusb_control_transfer error %d from FUNC: %s \n", retVal, __func__);
		for(i=0; i < sizeof(iobuffer); i++)
			printf(" %x ", iobuffer[i]);
		return FAILURE;
	}

	dfu_status->bStatus = iobuffer[0];
	dfu_status->bwPollTimeout[0] = iobuffer[1];
	dfu_status->bwPollTimeout[1] = iobuffer[2];
	dfu_status->bwPollTimeout[2] = iobuffer[3];
	dfu_status->bState = iobuffer[4];
	dfu_status->iString = iobuffer[5];

	return SUCCESS;
}

uint8_t sendcommand(int request, libusb_device_handle *devHandle)
{
	int retVal = 0;

	if(!devHandle) {
		printf("\n %s ==> devHandle is NULL\n", __func__);
		return FAILURE;
	}

	retVal = libusb_control_transfer(devHandle,
			(LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR),
			request, 1, 0, NULL, 0, 1000);

	if(retVal < 0) {
		printf("\n libusb_control_transfer error %d from FUNC: %s \n", retVal, __func__);
		return FAILURE;
	}

	return SUCCESS;
}

/*@Name: sendzerolengthpkt
 *@Description: This function sends zerolength packet to indicate firmware trasfer is complete.
 *@Returns: SUCCESS or FAILURE
 */
uint8_t sendzerolengthpkt(libusb_device_handle *devHandle)
{
	int retVal = 0;

	if(!devHandle) {
		printf("\n devHandle is NULL inside %s\n", __func__);
		return FAILURE;
	}

	retVal = libusb_control_transfer(devHandle,
			(LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR),
			USB_REQ_DFU_DNLOAD, 1, 0, NULL, 0, 1000);

	if(retVal < 0)
	{
		DEBUG_PRINT(("\n libusb_control_transfer error %d from FUNC: %s ", retVal, __func__));
		return FAILURE;
	}

	return SUCCESS;
}

/*@Name: senddata
 *@Description: This function sends the commad using libusb_control_transfer
 *Returns: SUCCESS or FAILURE
 */
uint8_t senddata(unsigned char *data, int bytes, libusb_device_handle *devHandle)
{
	int retVal = 0;

	if(!devHandle)
		return FAILURE;

	retVal = libusb_control_transfer(devHandle,
			(LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR),
			USB_REQ_DFU_DNLOAD, 1, 0, data, bytes, 1000);

	if(retVal < 0) {
		printf("\n libusb_control_transfer error %d from FUNC: %s \n", retVal, __func__);
		return FAILURE;
	}

	return SUCCESS;
}

/*@Name: do_download_prifwimg
 *@Description: This function opens,reads the Primary image file and write it to device.
 *@Returns: SUCCESS or FAILURE
 */
int do_download_prifwimg(char *filepath, libusb_device_handle *devHandle)
{
	FILE *fp;
	int64_t bytesread;
	unsigned char temp[DATA_PACKET_SIZE+10];

	DFU_STATUS dfu_status;

	uint32_t filesize;
	uint32_t totalframes;
	int64_t bytesleft = 0;
	int i = 0;
	float percent = 0.00;
	int firstfwlen, secondfwlen, fw_pos;
	long filelength;
	char *fwbin;
	int memerase = 1;

	fp = fopen(filepath, "rb");
	if(!fp) {
		printf("\n fopen error from FUNC: %s \n", __func__);
		return FAILURE;
	}
	fseek(fp, 0L, SEEK_END);
	filesize = ftell(fp);
	rewind(fp);

	fwbin = malloc(filesize);
	if (!fwbin) {
		return FAILURE;
	}
	fread(fwbin, 1, filesize, fp);
	fclose(fp);

	memcpy(&firstfwlen, &fwbin[9], 4);
	memcpy(&secondfwlen, &fwbin[firstfwlen + 0x0d + strlen("econIMG") + 1], 4);

	printf(ANSI_TEXT_BOLD);
	printf(ANSI_COLOR_RED "\n\n\t\t DO NOT UNPLUG THE DEVICE!" ANSI_COLOR_RESET "\n");
	fflush(stdout);
	setvbuf(stdout, NULL, _IONBF, 0);

	totalframes=(firstfwlen / DATA_PACKET_SIZE);
	if((filesize%DATA_PACKET_SIZE))
		totalframes++;

	filesize = (1+7+1+4+firstfwlen);
	for (fw_pos = (1+7+1+4);
		fw_pos < filesize; fw_pos+= DATA_PACKET_SIZE) {
		bytesread = ((filesize - fw_pos) > DATA_PACKET_SIZE)?
					DATA_PACKET_SIZE:(filesize - fw_pos);
		if(senddata(&fwbin[fw_pos], (int)bytesread, devHandle)) {
			if (memerase) {
				printf("\n Erasing Memory please wait ...\n");
				sleep(2);
				memerase = 0;
			}

			do {
				if( getstatus(&dfu_status, devHandle) == FAILURE) {
					printf("\n Failed....\n");
					 free(fwbin);
					 return 0;
				}
				usleep(10);
			}while(dfu_status.bState != DFU_STATE_dfuDNLOAD_IDLE);
		}
		percent = (100.00/(float)totalframes) * i++;
		printf("\r Updating Firmware [%3.2f%%]", percent);
		fflush(stdout);
	}
	printf("\n\033[F\033[J");
	printf(" Updating Firmware [100%%]\n");

	printf(ANSI_COLOR_GREEN   " Primary Firmware Updated"   ANSI_COLOR_RESET "\n");
	fflush(stdout);
	sendzerolengthpkt(devHandle);
	getstatus(&dfu_status, devHandle);
	free(fwbin);
	return SUCCESS;
}

/*@Name: do_download_secfwimg
 *@Description: This function opens,reads the secondary image file and write it to device.
 *@Returns : SUCCESS or FAILURE
 */
int do_download_secfwimg(char *filepath, libusb_device_handle *devHandle)
{
	FILE *fp;
	int64_t bytesread;
	unsigned char temp[DATA_PACKET_SIZE+10];

	DFU_STATUS dfu_status;

	uint32_t filesize;
	uint32_t totalframes;
	int64_t bytesleft = 0;
	int i = 0;
	float percent = 0.00;
	int firstfwlen, secondfwlen, sec_fw_pos;
	char *fwbin;
	int memerase = 1;

	fp = fopen (filepath, "rb");
	if(!fp) {
		printf("\n fopen error from FUNC: %s \n", __func__);
		return FAILURE;
	}
	fseek(fp, 0L, SEEK_END);
	filesize = ftell(fp);
	rewind(fp);

	fwbin = malloc(filesize);
	if (!fwbin) {
		return FAILURE;
	}
	fread(fwbin, 1, filesize, fp);
	fclose(fp);
	memcpy(&firstfwlen, &fwbin[9], 4);
	memcpy(&secondfwlen, &fwbin[firstfwlen + 0x0d + strlen("econIMG") + 1], 4);

	printf(ANSI_TEXT_BOLD);
	printf(ANSI_COLOR_RED "\n\n\t\t DO NOT UNPLUG THE DEVICE!" ANSI_COLOR_RESET "\n");
	fflush(stdout);
	setvbuf(stdout, NULL, _IONBF, 0);

	totalframes=(secondfwlen / DATA_PACKET_SIZE);
	if((filesize%DATA_PACKET_SIZE))
		totalframes++;

	for (sec_fw_pos = (1+7+1+4+firstfwlen+7+1+4);
		sec_fw_pos < filesize; sec_fw_pos+= DATA_PACKET_SIZE) {
		bytesread = ((filesize - sec_fw_pos) > DATA_PACKET_SIZE)?
					DATA_PACKET_SIZE:(filesize - sec_fw_pos);
		if(senddata(&fwbin[sec_fw_pos], (int)bytesread, devHandle)) {
			if (memerase) {
				printf("\r Erasing Memory please wait ...");
				sleep(4);
				memerase = 0;
			}
			do {
				if( getstatus(&dfu_status, devHandle) == FAILURE) {
					 free(fwbin);
					 return 0;
				}
				usleep(10);
			}while(dfu_status.bState != DFU_STATE_dfuDNLOAD_IDLE);
		}
		percent = (100.00/(float)totalframes) * i++;
		printf("\r Updating Firmware [%3.2f%%]", percent);
		fflush(stdout);
	}
	printf("\n\033[F\033[J");
	printf("\n Updating Firmware [100%%]\n");

	printf(ANSI_COLOR_GREEN   " Secondary Firmware Updated"   ANSI_COLOR_RESET "\n");
	fflush(stdout);
	sendzerolengthpkt(devHandle);
	getstatus(&dfu_status, devHandle);
	free(fwbin);
	return SUCCESS;
}

/*@Name: dodownload
 *@Description: this function reads the image file and sends it to device
 *@Returns: SUCCESS or FAILURE
 */
int dodownload(char *filepath, libusb_device_handle *devHandle)
{
	FILE *firmwarefp;
	int64_t bytesread;
	unsigned char temp[DATA_PACKET_SIZE+10];

	DFU_STATUS dfu_status;
	uint32_t filesize;
	uint32_t totalframes;
	int64_t bytesleft = 0;
	int i = 0;
	float percent = 0.00;
	int memerase = 1;
	int pad_size = 0;
	firmwarefp = fopen (filepath, "rb");

	if(!firmwarefp) {
		printf("fopen error from FUNC: %s \n", __func__);
		return FAILURE;
	}

	printf("\n\nReading file.. \n");

	fseek(firmwarefp,0,SEEK_END);
	filesize = ftell(firmwarefp);
	printf("\n\nImage file size = %d bytes", filesize);
	fseek(firmwarefp,0,SEEK_SET);
	fflush(stdout);

	totalframes=(filesize / DATA_PACKET_SIZE);
	if((filesize%DATA_PACKET_SIZE))
		totalframes++;

	bytesleft = filesize;

	printf(ANSI_TEXT_BOLD);
	printf(ANSI_COLOR_RED "\n\n\t\t DO NOT UNPLUG THE DEVICE!" ANSI_COLOR_RESET "\n");

	fflush(stdout);

	setvbuf(stdout, NULL, _IONBF, 0);

	for(i = 0;i < totalframes; i++) {
		if(bytesleft > DATA_PACKET_SIZE) {
			bytesread = fread(temp, 1, DATA_PACKET_SIZE, firmwarefp);
			if(bytesread != DATA_PACKET_SIZE) {
				printf("File Read Failed\r\n");
			}
		}else {
			memset(temp,sizeof(temp),0);
			bytesread = fread(temp, 1, bytesleft, firmwarefp);
			if(bytesread != bytesleft)
			{
				printf("File Read Failed \r\n");
			}
			if(!(bytesread % 64))
				pad_size = 8;
		}
		bytesleft -= bytesread;

		if(senddata(temp, (int)bytesread+pad_size, devHandle)) {
			if (memerase) {
				printf("\r Erasing Memory please wait ...");
				sleep(3);
				memerase = 0;
			}
			do
			{
				if( getstatus(&dfu_status, devHandle) == FAILURE)
				{
					 fclose(firmwarefp);
					 return 0;
				}
				usleep(10);
			} while(dfu_status.bState != DFU_STATE_dfuDNLOAD_IDLE);
		}
		else
		{
			fclose(firmwarefp);
			return FAILURE;
		}

		// Print the percentage completed

		percent = (100.00/(float)totalframes) * i;
		printf("\r Updating Firmware [%3.2f%%]", percent);
		fflush(stdout);
	}
	printf("\n\033[F\033[J");
	printf(" Updating Firmware [100%%]\n");

	printf(ANSI_COLOR_GREEN   "\n Firmware Updated"   ANSI_COLOR_RESET "\n");
	fflush(stdout);

	fclose(firmwarefp);
	return SUCCESS;
}

/*
 *@name: open_xu
 *@parameters: void
 *@Description: function to open extension unit
 *@Return: SUCCESS or FAILURE
 */
int open_xu(const char *str)
{
    struct udev *udev;
    struct udev_device *dev,*pdev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    /* create udev object */
    udev = udev_new();
    if (!udev)
    {
        perror("\nCannot create udev context.\n");
        return FAILURE;
    }
    /* create enumerate object */
    enumerate = udev_enumerate_new(udev);
    if (!enumerate)
    {
        perror("\nCannot create enumerate context.\n");
        return FAILURE;
    }
    udev_enumerate_add_match_subsystem(enumerate, str);
    udev_enumerate_scan_devices(enumerate);

    /* fillup device list */
    devices = udev_enumerate_get_list_entry(enumerate);
    if (!devices)
    {
        perror("\nFailed to get device list");
        return FAILURE;
    }
    udev_list_entry_foreach(dev_list_entry, devices)
    {
        const char *path,*sNo=NULL;

        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);

        pdev = udev_device_get_parent_with_subsystem_devtype(dev,"usb","usb_device");
        if(!pdev)
            continue;
        const char *udev_device = udev_device_get_devnode(dev);

				sNo=udev_device_get_sysattr_value(pdev,"idProduct");
        if(sNo==NULL)
            continue;

        if((strcmp(sNo,"c123")==0)||(strcmp(sNo,"c129")==0) ||(strcmp(sNo,"0135")==0) || (strcmp(sNo,"df02")==0) || (strcmp(sNo,"c05c")==0) )
        {
            fd=open(udev_device,O_RDWR|O_NONBLOCK);
						if(fd<0)
                perror("Error open");
	    			else
	    			{
							udev_enumerate_unref(enumerate);// free enumerate
    					udev_unref(udev);               // free udev
							return SUCCESS;
						}
    		}
			}
    udev_enumerate_unref(enumerate);// free enumerate
    udev_unref(udev);               // free udev
    return FAILURE;
}

/*
 *@name: hyperyon_read_firmware_version
 *@parameters: struct version_s - structure for holding firmware version numbet.
 *@Description: function to switch hyperyon devices to dfu mode via extension unit
 *@Return: SUCCESS or FAILURE
 */

int hyperyon_read_firmware_version(version_s *version)
{
    struct uvc_xu_control_query xquery;
    __u16 size = 0;
    __u8 outputValues[4];
    int ret=-1;
    memset(&xquery, 0, sizeof(xquery));

    // To allocate a sufficiently large buffer and set the buffer size to the correct value
    xquery.query = UVC_GET_LEN;
    xquery.size = 2;
    xquery.selector = V4L2_CID_XU_FW_VERSION;
    xquery.unit = EXTENSION_UNIT_ID;
    xquery.data = (__u8 *)&size;

    ret =ioctl(fd,UVCIOC_CTRL_QUERY,&xquery);
    if(ret<0)
			return FAILURE;

    memset(&xquery, 0, sizeof(xquery));

    // get value from camera
    xquery.query = UVC_GET_CUR;
    xquery.size = size;
    xquery.selector = V4L2_CID_XU_FW_VERSION;
    xquery.unit = EXTENSION_UNIT_ID;
    xquery.data = (__u8 *)&outputValues;


    ret =ioctl(fd,UVCIOC_CTRL_QUERY,&xquery);
    if(ret<0)
			return FAILURE;

    version->MajorVersion  = outputValues[0];
    version->MinorVersion1 = outputValues[1];
    version->MinorVersion2 = outputValues[2];	// SDK Version
    version->MinorVersion3 = outputValues[3];	// SVN Version

    return SUCCESS;
}
/*
 *@name: switch_to_dfu_mode
 *@Description: function to switch hyperyon devices to dfu mode via extension unit
 *@Return: SUCCESS or FAILURE
 */
int switch_to_dfu_mode()
{
    struct uvc_xu_control_query xquery;
    __u16 size = 0;
		__u8 setVal= 0x01;
    int ret=-1;
    memset(&xquery, 0, sizeof(xquery));

    xquery.query = UVC_GET_LEN;
    xquery.size = 2;
    xquery.selector = V4L2_CID_XU_DFUMODE;
    xquery.unit = EXTENSION_UNIT_ID;
    xquery.data = (__u8 *)&size;

    ret =ioctl(fd,UVCIOC_CTRL_QUERY,&xquery);
    if(ret<0)
	return FAILURE;

    memset(&xquery, 0, sizeof(xquery));

    xquery.query = UVC_SET_CUR;
    xquery.size = size;
    xquery.selector = V4L2_CID_XU_DFUMODE;
    xquery.unit = EXTENSION_UNIT_ID;
    xquery.data =  &setVal;

    ret =ioctl(fd,UVCIOC_CTRL_QUERY,&xquery);

    if(ret<0)
			return FAILURE;

    return SUCCESS;
}
