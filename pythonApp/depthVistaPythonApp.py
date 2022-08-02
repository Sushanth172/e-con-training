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

        # Initialize(API) from the library
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



''' DRIVER FUNCTION '''
if __name__ == "__main__":
    intro()
    main = MainClass()


