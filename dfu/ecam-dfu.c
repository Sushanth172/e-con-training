#include <errno.h>
#include <sys/stat.h>
#include "ecam-dfu.h"

#define MAX_FILE_PATH 4096
 int retry;
int main(int argc, char *argv[])
{
	int retVal = 0;
	int detach_interface = 0;

	libusb_device *devPtr = NULL;
	libusb_device_handle *devHandle	= NULL;

	device_id_s dev;
	version_s version;

	printf("\n\n\t\tE-CONSYSTEMS's Firmware Update Application %s\n",VERSION_N);
  printf("=========================================================================================\n");
	printf(" Checking for devices...\n");

reenum:
	//  find number of cypress mode devices

	updateCypressModeDevices();

	sleep (3);

	int econDevCount = 0;
	int option;

	// Get the number of econ cameras[both DFU mode and UVC mode ]

	econDevCount = getNumberOfEconDevices();
	if(econDevCount == 0){
      for( ; ; ){
        printf("\n No e-con devices found.\n");
        printf("\n 1 - Re-enumerate\n");
        printf(" 2 - Exit\n");

        printf("\n Please Enter the option: ");
    	  scanf("%d", &option);

       while(getchar() != '\n'){}

       if(option < 1  || option > 2){
            printf("\n Invalid option.\n");
            continue;
          }
          break;
      }
    	if(option == 2){	        // Close the application
    		exit(0);
    	}else if(option == 1){		// Reenumerate - Again search and list the econ cameras.
    		goto reenum;
    	}
	}
  else
  {
    printf("\n List of e-con cameras. Choose a camera\n\n");
  }

	int usbDeviceIndexValues[econDevCount];
	// Main menu

	// Get the list of econ cameras[both DFU mode and UVC mode ]
	if(enumerate_all_econ_devices(usbDeviceIndexValues, econDevCount) != SUCCESS){
		printf("\n Please make sure you are launching the application with sudo.\n");
		return 0;
	}

	printf(" %d - Re-enumerate\n", econDevCount+1);
	printf(" %d - Exit\n", econDevCount+2);

	printf("\n Please Enter the option: ");
	scanf("%d", &option);
	while(getchar() != '\n'){}
	if(option < 1 || option > (econDevCount+2)){    // If invalid option is chosen, reenumerate list
		printf("\n Invalid option.\n");
		freeDevBusDetails(econDevCount);
		goto reenum;
	}else if(option == econDevCount+2){	        // Close the application
		freeDevBusDetails(econDevCount);
		exit(0);
	}else if(option == econDevCount+1){		// Reenumerate - Again search and list the econ cameras.
		freeDevBusDetails(econDevCount);
		goto reenum;
	}

	// sub menu -  update fw (or) read fw version
	int subOption;

subMenu:
		for( ; ; ){
		printf("\n 1 - Update Firmware\n");
		printf(" 2 - Firmware version\n");
        printf(" 3 - Reset\n");
		printf(" 4 - Main menu\n");

		printf(" Please Enter the option: ");
		scanf("%d", &subOption);

		while(getchar() != '\n'){}

		if(subOption < UPDATE_FIRMWARE  || subOption > GOTO_MAIN){
			printf("\n Invalid option.\n");
			continue;
		}
		break;
	}

	switch(subOption){
		case GOTO_MAIN:{        // Go to main menu
			printf("\n Enumerating econ devices......\n");
			freeDevBusDetails(econDevCount);
			goto reenum;
		}break;

		case UPDATE_FIRMWARE:{	 // update firmware option
			devPtr = NULL; devHandle = NULL;
			// Initialize libusb here
			retVal = usb_init();

			if(retVal < 0) {
				printf ("%d  Init Error ",retVal); //there was an error
				freeDevBusDetails(econDevCount);
				return -1;
			}

			char imagePath[MAX_FILE_PATH];
      int imageFilesize=0;
			while(1)
			{
				printf("\n Please enter the firmware image path: ");
				scanf("%s",imagePath);
				FILE *file;
				if (file = fopen(imagePath, "r"))
				{
          fseek(file,0,SEEK_END);
          imageFilesize = ftell(file);
          // fseek(file,0,SEEK_SET);
					fclose(file);
					break;
				}
				else
				{
					printf("\n Warning! File is not available.\n");
					continue;
				}
			}

			// update the firmware
			retVal = updatefirmware(&dev, imagePath, &detach_interface, *(usbDeviceIndexValues+(option-1)) , TRUE, &devHandle, &devPtr, econDevCount,imageFilesize);

			// close usb device
			usb_close(devHandle, devPtr);
			if(retVal==SUCCESS)
			{
				if(usb_init()<0)
				{
					printf (" %d  Init Error ",retVal); //there was an error
					freeDevBusDetails(econDevCount);
					return -1;
				}
				retry = FIND_RETRY;
				do
				{
					retVal = find_econ_device(&dev, FALSE, NIL_INDEX , TRUE, &devPtr, econDevCount);
          if(retVal!=SUCCESS)
              sleep(1);
					--retry;
				}while((retry!=0)&&retVal==SUCCESS);
				printf("\n Enumerating econ devices......\n");
			}
			else
				printf("\n\n Firmware update failed!!\n");
			sleep(3);

			freeDevBusDetails(econDevCount);
			goto reenum;	// After updating firmware , again reenumerate the devices
		}break;

		case READ_FW_VERSION:{ // read fw version option

			devPtr = NULL; devHandle = NULL;

			// Initialize libusb here
			retVal = usb_init();

			if(retVal < 0) {
				printf ("\n %d  Init Error ",retVal); //there was an error
				freeDevBusDetails(econDevCount);
				return -1;
			}

			// find the econ device to read fw version
			retVal = find_econ_device(&dev, FALSE, *(usbDeviceIndexValues+(option-1)) , TRUE, &devPtr, econDevCount);

			if (retVal == PRESENT) {
				if (dev.idProduct == DFU_PID_DF01 ||dev.idProduct == DFU_PID_DF00 || dev.idProduct == ECAM51B_USB_DFU_PID) {
					printf("\n Device is in DFU mode - cannot read firmware version.\n\n");

				}
				else if(dev.idProduct == NXP_PID){
				  printf("\n Device is in NXP mode - cannot read firmware version.\n\n");
				}else {
					unsigned char *strDesc = NULL;
					strDesc = (unsigned char *) calloc(PROD_LEN, sizeof(unsigned char));

					retVal = readfirmwareversion(&dev, &version, &strDesc, *(usbDeviceIndexValues+(option-1)),
							TRUE, &devHandle, devPtr);


					switch(retVal) {
						case ABSENT:
							printf(" No See3CAM found \n");
							break;

						case SUCCESS:
							printf("\n Current Firmware version of %s camera is ==>  %d.%d.%d.%d \n\n\n", strDesc, version.MajorVersion, version.MinorVersion1,
									version.MinorVersion2, version.MinorVersion3);

							break;

						default:
							printf("\nERROR in getting firmware version.\n\n\n");
							break;

					}
					if(strDesc)
					{
						free(strDesc);
						strDesc = NULL;
					}

				}
			}
			// close usb device
			usb_close(devHandle, devPtr);
			goto subMenu;// goto submenu again

		}break;

    case RESET:{ // reset device option

      devPtr = NULL; devHandle = NULL;

      // Initialize libusb here
      retVal = usb_init();

      if(retVal < 0) {
        printf ("\n %d  Init Error ",retVal); //there was an error
        freeDevBusDetails(econDevCount);
        return -1;
      }

      // Returns the Number of e-con devices(Both DFU & Streaming Devices)
      retVal = find_econ_device(&dev, FALSE, *(usbDeviceIndexValues+(option-1)) , TRUE, &devPtr, econDevCount);

      if (retVal == PRESENT) {
        if (dev.idProduct == DFU_PID_DF01 ||dev.idProduct == DFU_PID_DF00 || dev.idProduct == ECAM51B_USB_DFU_PID) {
				unsigned char *strDesc = NULL;
				strDesc = (unsigned char *) calloc(PROD_LEN, sizeof(unsigned char));
				// retVal = resetDfuDevice(&dev, &strDesc, *(usbDeviceIndexValues+(option-1)), &devHandle, devPtr);
				// retVal = resetDfuDevice(&dev, &strDesc, *(usbDeviceIndexValues+(option-1)), &devHandle, devPtr);


	            retVal = resetDfuDevice(&dev, &detach_interface, &devHandle,  &devPtr);

				if(retVal==SUCCESS)
				{
					printf(ANSI_TEXT_BOLD);
					printf(ANSI_COLOR_GREEN "\n\n\t\tDEVICE RESET SUCCESSFULLY" ANSI_COLOR_RESET "\n");
					if(usb_init()<0)
					{
						printf (" %d  Init Error ",retVal); //there was an error
						freeDevBusDetails(econDevCount);
						return -1;
					}
					retry = FIND_RETRY;
					do
					{
						retVal = find_econ_device(&dev, FALSE, NIL_INDEX , TRUE, &devPtr, econDevCount);
						if(retVal!=SUCCESS)
							sleep(1);
						--retry;
					}while((retry!=0)&&retVal==SUCCESS);
					printf("\n Enumerating e-con devices......\n");
				}
				else
				{
					printf(ANSI_TEXT_BOLD);
					printf(ANSI_COLOR_RED "\n\n\t\t RESET DEVICE FAILED!!" ANSI_COLOR_RESET "\n");
				}
				// sleep(3);

				freeDevBusDetails(econDevCount);
				goto reenum;	// After reseting , again reenumerate the device.
				// printf("\n Device is in DFU mode - cannot reset device.\n\n");
        }
        else if(dev.idProduct == NXP_PID){
          printf("\n Device is in NXP mode - cannot reset device.\n\n");
        }
        else if(dev.idProduct == HYPERYON_PID){
          printf("\n Hyperyon camera - Reset not supported.\n\n");
        }
        else {
          unsigned char *strDesc = NULL;
					strDesc = (unsigned char *) calloc(PROD_LEN, sizeof(unsigned char));

          retVal = reset_device(&dev, &strDesc, *(usbDeviceIndexValues+(option-1)), &devHandle, devPtr);

          // close usb device
          usb_close(devHandle, devPtr);
          if(retVal==SUCCESS)
          {
            printf("\n Reset device success\n");
            if(usb_init()<0)
            {
              printf (" %d  Init Error ",retVal); //there was an error
              freeDevBusDetails(econDevCount);
              return -1;
            }
            retry = FIND_RETRY;
            do
            {
              retVal = find_econ_device(&dev, FALSE, NIL_INDEX , TRUE, &devPtr, econDevCount);
              if(retVal!=SUCCESS)
                  sleep(1);
              --retry;
            }while((retry!=0)&&retVal==SUCCESS);
            printf("\n Enumerating e-con devices......\n");
          }
          else
		  {
			printf(ANSI_TEXT_BOLD);
		  	printf(ANSI_COLOR_RED "\n\n\t\t RESET DEVICE FAILED!!" ANSI_COLOR_RESET "\n");
		  }
		  	
          // sleep(3);

          freeDevBusDetails(econDevCount);
          goto reenum;	// After reseting , again reenumerate the device.
        }

      }

    }break;
		freeDevBusDetails(econDevCount);
	}
	return 0;
}
