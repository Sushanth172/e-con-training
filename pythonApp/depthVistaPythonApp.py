import cv2
import ctypes
from ctypes import *

import hid


def intro():
    print(" E-con's Depth Vista OpenCV Python Application ".center(100, "*"))
    print('OpenCV Python App Version = 1.0.3'.center(100, " "))
    print("Running in Linux Platform".center(100, " "))

class MainClass:

    '''
        Main Class: The initiator class. The program starts from here.
    '''
    def __init__(self):
        sharedLibary = "libDepthVistaSDK.so"
        depthVistaLib = ctypes.CDLL(sharedLibary)

        # CAN open API from library
        result = depthVistaLib.Initialize
        result.restype = ctypes.c_int
        if result() == 0:
            print("\nFAILED INITIALIZE\n")
        # self.vid = None
        # self.pid = None
        # self.device_path = None
        # self.device_name = None
        # self.Is_HID_Opened = False

        deviceCountResult = depthVistaLib.GetDeviceCount
        deviceCountResult.restype = ctypes.c_int
        deviceCountResult.argtypes = [
            ctypes.POINTER(ctypes.c_uint32)
        ]
        numberOfDevices = ctypes.c_uint32 ()
        if deviceCountResult(ctypes.byref(numberOfDevices)) == 1:
            print(f'\nNumber Of Depth Vista(TOF) Device Connected: {numberOfDevices.value}')
        else:
            print("\nFAILED IN GETTING DEVICE COUNT\n")





    # def mainMenu(self):
    #     mainMenuOptions={
    #         0: self.main_menu_exit,
    #         1: self.main_menu_init,
    #         2: self.uvc_obj.change_uvc_control,
    #         3: self.format.change_format,
    #         4: self.still_capture,
    #         5: self.hid_control_menu,
    #      }
    #      while True:
    #          print("\n0.Exit")
    #          print("\n1.Back")
    #          print("\n2.Streaming Mode")
    #          print("\n3.Depth Range")
    #          print("\n4.Planarization")
    #          print("\n5.Capture Frames")
    #          print("\n6.Unique Id")
    #          print("\n7.Read Firmware Version")
    #
    #          print("\nEnter Your Option:")
    #          option = int(input())
    #
    #          if(option == 0):
    #              print("\nEXIT")
    #              return FALSE
    #          elif(option == 1):
    #              print("\Back")
    #          elif(option == 2):
    #              print("\nStreaming Mode")
    #          elif(option == 3):
    #              print("\nDepth Range")
    #          elif(option == 4):
    #              print("\nPlanarization")
    #          elif(option == 5):
    #              print("\nCapture Frames")
    #          elif(option == 6):
    #              print("\nUnique Id")
    #          elif(option == 7):
    #              print("\nRead Firmware Version")
    #          else:
    #              print("\nPick a Relevent Choice of Camera Properties:")
    #              choice = int(input())
    #
    # def selectStreamingMode(self):
    #     streamingModeOptions={
    #         0: self.main_menu_exit,
    #         1: self.main_menu_init,
    #         2: self.uvc_obj.change_uvc_control,
    #         3: self.format.change_format,
    #         4: self.still_capture,
    #         5: self.hid_control_menu,
    #         6: self.main_menu_exit,
    #         7: self.main_menu_init,
    #         8: self.uvc_obj.change_uvc_control,
    #         9: self.hid_control_menu,
    #         10: self.main_menu_exit,
    #         11: self.main_menu_init,
    #     }
    #      print("\nTotal Number of Streaming Modes Supported By the Camera:\t9")
    #
    #      print("\n0.Exit")
    #      print("\n1.Back")
    #      print("\n2.Main Menu")
    #      print("\n3.Depth IR Mode")
    #      print("\n4.Depth Mode")
    #      print("\n5.IR Mode")
    #      print("\n6.Depth IR RGB(VGA) Mode")
    #      print("\n7.Depth IR RGB(HD) Mode")
    #      print("\n8.RGB(VGA) Mode")
    #      print("\n9.RGB(HD) Mode")
    #      print("\n10.RGB(Full HD) Mode")
    #      print("\n11.RGB(1200p) Mode")
    #
    #        print("\nSelect Any Streaming Mode")
    #        choice=int(input())
    #        #print("\nPick a Relevent Choice of Camera Properties")
    #
    #        if(choice==0):
    #            print(exit)
    #        elif(choice == 1):
    #            print("Back")
    #        elif(choice == 2):
    #            print("MainMenu")
    #        elif(choice == 3):
    #            print("\n3.Depth IR Mode")
    #        elif(choice == 4):
    #            print("\n4.Depth Mode")
    #        elif(choice == 5):
    #            print("\n5.IR Mode")
    #        elif(choice == 6):
    #            print("\n6.Depth IR RGB(VGA) Mode")
    #        elif(choice == 7):
    #            print("\n7.Depth IR RGB(HD) Mode")
    #        elif(choice == 8):
    #            print("\n8.RGB(VGA) Mode")
    #        elif(choice == 9):
    #

     # '''
     #    Method name: main_menu_exit
     #    description: this method is called before program exists. This method
     #    releases cap, stop display and de_init hid
     # '''

     # def main_menu_exit(self):
     #    self.display2.stop_display()
     #    self.display2.Kill_Display_thread()
     #    # When everything done, release the capture
     #    self.cap.release()
     #    cv2.destroyAllWindows()
     #    self.hid.deinit_hid()
     #    exit(0)




''' DRIVER FUNCTION '''
if __name__ == "__main__":
    intro()
    main = MainClass()


    # '''
    #     THIS IS THE FUNCTION TO INITIALIZE ALL THE APIS
    # '''
    # def initializeCam(self):
    #     if((self.depthVistaLib.Initialize)<0):
    #         print("INITIALIZING THE APIs IS SUCCESS")
    #     else:
    #         print("INITIALIZATION IS FAILED")
    #
    # '''
    #     THIS FUNCTION IS TO GET THE COUNT OF TOF DEVICES CONNECTED
    # '''
    # def get_device_count(self):
    #     if(self.GetDeviceCount(deviceCount)>0):
    #         print("Number Of Depth Vista(TOF) Device Connected: {0}".format(device_count))
    #     else:
    #         print("Get Device Count Failed")
    #
    # def get_device_list_info(self):
    #     print("\nList Device Info")
