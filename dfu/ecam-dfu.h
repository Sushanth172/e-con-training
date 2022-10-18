#define BINARY 0 // By default this will be 0,change this to 1 while making AppImages(binary)
#define VERSION_N		 "1.0.0.9"

#include <libusb-1.0/libusb.h>	// libusb
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include "usb_dfu.h"
#include <linux/hidraw.h>

// Main menu
enum {
	UPDATE_FIRMWARE = 1,
	READ_FW_VERSION,
	RESET,
	GOTO_MAIN
};

// Firmware version fields
typedef struct{
	uint8_t MajorVersion;
	uint8_t MinorVersion1;
	uint16_t MinorVersion2;
	uint16_t MinorVersion3;	// Variables to get Firmware version
}version_s;

// Device specific details
typedef struct{
	uint16_t idVendor;
	uint16_t idProduct;
	uint8_t iProduct;
}device_id_s;

int usb_init();

int find_econ_device(device_id_s *device, uint8_t list, int index, int no_arg, libusb_device **devPtr, int econDevCount);

int readfirmwareversion(device_id_s *device, version_s *version, unsigned char **strDesc, int index, int no_arg, libusb_device_handle **devHandle, libusb_device *devPtr);

int claimdevice(device_id_s device, uint8_t interface_class, unsigned char **strDesc,
				int *detach_interface, libusb_device_handle **devHandle, libusb_device *devPtr);

int updatefirmware(device_id_s *device, char *image_path, int *detach_interface, int index, int no_arg,	libusb_device_handle **devHandle,libusb_device **devPtr, int econDevCount,int imageFilesize);

int reset_device(device_id_s *device, unsigned char **strDesc, int index, libusb_device_handle **devHandle, libusb_device *devPtr);

int resetDfuDevice(device_id_s *device, int *detach_interface,libusb_device_handle **devHandle, libusb_device **devPtr);

int change_to_dfu_mode(device_id_s device, int *detach_interface,
				 libusb_device_handle **devHandle, libusb_device *devPtr, char *image_path);

int change_to_dfu_idlestate(libusb_device_handle *devHandle);

int device_close(libusb_device_handle *devHandle, int dfu_status, int detach_interface);

int dodownload(char * filepath, libusb_device_handle *devHandle);

int8_t getstatus( DFU_STATUS *dfu_status, libusb_device_handle *devHandle);

uint8_t sendcommand(int request, libusb_device_handle *devHandle);

uint8_t sendzerolengthpkt(libusb_device_handle *devHandle);

uint8_t senddata(unsigned char *data, int bytes, libusb_device_handle *devHandle);

void usb_close(libusb_device_handle *devHandle, libusb_device *devPtr);

int check_file(char * image_path);

unsigned gettickcount(void);

// Added by Sankari:17 May 2018 - Find number of cypress mode devices
int findNoOfcypressModeDevices();

// Update bootloaders to all cypress mode devices
int updateCypressModeDevices();

// Find number of Econ devices
int getNumberOfEconDevices();

// Enumerate all devices with details. Get econ devices bus information, camera name and camera product id
int enumerate_all_econ_devices(int *usbDeviceIndexValues, int econDevCount);

// econ devices bus details will be cleared
void freeDevBusDetails(int econDevCount);

// Load bootloader file in cypress mode
int fx3_usbboot_download(const char *filename);

// To write primary firmware image in device
int do_download_prifwimg(char *filepath, libusb_device_handle *devHandle);

// To write secondary (UVC) firmware image in device
int do_download_secfwimg(char *filepath, libusb_device_handle *devHandle);



/* ------------------- MACROS FOR ECON-DFU-UTIL -------------------*/

#define ECON_VID 		9568
#define VID_N 			"e-con Systems"

#define HID_INTERFACE		0x03
#define DFU_INTERFACE		0xFF
#define NXP_INTERFACE		0x00
#define	DFU_PID_DF01		0xDF01 // dfu mode from cypress
#define DFU_PID_DF00		0xDF00 // dfu mode which does not involve cypress


#define FIND_RETRY   5
#define BUFFER_LENGTH		250
#define PROD_LEN		256
#define MAX_PATH		1024

#define IN 0x85 // EP 5 IN
#define OUT 0x06 // EP 6 OUT

#define READFIRMWAREVERSION 	0x40
#define FIRMWAREUPDATE		0x30
#define TIMEOUT			2000
#define DATA_PACKET_SIZE	(1024*1)

#define FALSE			0
#define TRUE			1

#define SUCCESS			2
#define FAILURE			-1

#define PRESENT			4
#define ABSENT			5

#define BOOTLOADERECON_VID	0x04B4
#define BOOTLOADERECON_PID	0x00F3

#define NIL_INDEX		-1

#define FCHECK_LEN		2

#define DFU_FLASH_MODE		0xA5C11		// Magic Number

#define ANSI_TEXT_BOLD		"\033[1m"
#define ANSI_COLOR_RED		"\x1b[31m"
#define ANSI_COLOR_GREEN	"\x1b[32m"
#define ANSI_COLOR_RESET	"\x1b[0m"

#define IMG_TYPE_1 0x01
#define IMG_TYPE_2 0x02
#define IMG_TYPE_3 0x03

#undef DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(x) printf x
#else
#define DEBUG_PRINT(x) do {} while (0)
#endif

enum {
	DFU_INPUT_FIRMWARE_OLD_TYPE = 0x861986,
	DFU_INPUT_FIRMWARE_NEW_TYPE,
	DFU_INPUT_FIRMWARE_NOT_PRESENT,
};
/*Declaration for hyperyon device*/
#include <libv4l2.h>
#include <sys/ioctl.h>
#include <linux/uvcvideo.h>
#include <libudev.h>
enum uvc_req_code {
  UVC_RC_UNDEFINED = 0x00,
  UVC_SET_CUR = 0x01,
  UVC_GET_CUR = 0x81,
  UVC_GET_MIN = 0x82,
  UVC_GET_MAX = 0x83,
  UVC_GET_RES = 0x84,
  UVC_GET_LEN = 0x85,
  UVC_GET_INFO = 0x86,
  UVC_GET_DEF = 0x87
};

#define EXTENSION_UNIT_ID        3
#define V4L2_CID_XU_FW_VERSION  0x07
#define V4L2_CID_XU_DFUMODE     0x05
#define HYPERYON_PID    0xc123
#define HYPERYON_DUAL_STREAM_PID 0xc129

#define IMG_HYPERYON 0x57

int hyperyon_read_firmware_version(version_s *version);
int switch_to_dfu_mode();
int open_xu(const char *str);
extern int retry;

/*Declaration for ecam51b_USB device*/
#define NXP_VID 									0x1fc9
#define NXP_PID										0x0135
#define ECAM51B_USB_ECON_PID 			0xC05C
#define ECAM51B_USB_DFU_PID  			0Xdf02
#define SEE3CAM160_PID						0xc400
#define PACKET_SIZE 							1025
#define ERROR_STATUS 							0x0505
#define WRITE_FILE 								0x0404
#define JUMP_ADDRESS 							0x0B0B
#define MAGIC_NUMBER 							0x0151
#define MEMORY_ADDRESS 						0x20001000
#define CMD_OUT 									1
#define MAX_ARG 									16
#define ECAM51B_USB_PACKET_SIZE 	1016

#define CMD_TAG_GETPROPERTY  				0x07
#define CMD_TAG_FILLMEMORY    			0x05
#define CMD_TAG_CONFIGMEMORY  			0x11
#define CMD_TAG_FLASH_ERASE_REGION  0x02
#define CMD_TAG_WRITEMEMORY 				0x04
#define CMD_TAG_SETPROPERTY  				0x0c

#define RESPONSE1_DATASIZE					5
#define RESPONSE2_DATASIZE					65
#define WRITE_DATASIZE							17
#define BLHOST_READ_DATASIZE 				1016

#define UVC_XU_HW_CONTROL_REGISTER          0x04
#define UVC_XU_HW_CONTROL_I2C_DEVICE        0x05
#define UVC_XU_HW_CONTROL_SENSOR            0x10
#define REG_CONTROL_WRITE_REGISTER			(0x1 << 5)
#define SENSOR_MODE_SELECT_REG_ADDR         0x8102

#define EEPROM_51B_FWV_REG_ADDRESS1         0x0094
#define EEPROM_51B_FWV_REG_ADDRESS2         0x0095
#define EEPROM_FWV_REG_ADDRESS1				0x0096
#define EEPROM_FWV_REG_ADDRESS2				0x0097
#define EEPROM_FWV_REG_ADDRESS3				0x0098
#define EEPROM_FWV_REG_ADDRESS4				0x0099
#define EEPROM_DEVICE_ADDRESS_FOR_READ_OPERATION	0xA0

struct timeval tv;
fd_set rfds;

struct cmd
{
		uint8_t rpt_header[4];
		uint8_t cmd_header[4];
		uint32_t arguments[MAX_ARG];
}cmdpkt;

int ecam51bUSB_prepare_sdphostBuffer(unsigned int command_code,unsigned int memory_address,unsigned char *g_out_packet_buf,unsigned char *g_in_packet_buf);

bool ecam51bUSB_prepare_blhostBuffer(unsigned int tag,int flag, int no_arg,uint32_t arg1,uint32_t arg2,uint32_t arg3);

bool send_hid_command(void *outBuf,int len, unsigned int check_response,unsigned char *g_in_packet_buf);

int ecam51bUSB_writeBootloader(unsigned char *g_in_packet_buf);

bool ecam51bUSB_writeImage(unsigned char *imagePath,unsigned char *g_in_packet_buf);

bool ecam51bUSB_write_data(unsigned int len,unsigned char *g_out_packet_buf);

bool ecam51bUSB_read_data(unsigned int len,unsigned char *g_in_packet_buf);

bool getControlValue(__u8 controlId, __u8 queryType, uint numberOfValues, __u8 *outputValues);

unsigned char reg_read_eeprom(unsigned int reg_address);

bool ecam51bUSB_readfirmware(version_s *version);

bool ecam51bUSB_reset_device();
