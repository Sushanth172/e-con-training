import cv2
import ctypes
from ctypes import *

from typing import NamedTuple
import hid

def intro():
    print(" E-con's Depth Vista OpenCV Python Application ".center(100, "*"))
    print('OpenCV Python App Version = 1.0.3'.center(100, " "))
    print("Running in Linux Platform".center(100, " "))


class DeviceInfo(ctypes.Structure):
    attributes = [
                  ('deviceName',ctypes.c_wchar_p),    #c_wchar_p is the pointer to the string
                  ('vid',ctypes.c_wchar_p),
                  ('pid',ctypes.c_wchar_p),
                  ('devicePath',ctypes.c_wchar_p),
                  ('serialNo',ctypes.c_wchar_p)
                 ]
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


        '''
            GETTING DEVICE LIST INFORMATION
        '''
        listDevicesResult = depthVistaLib.GetDeviceInfo
        listDevicesResult.argtypes = [ ctypes.c_uint32, ctypes.POINTER(DeviceInfo)]
        listDevicesResult.restype = ctypes.c_int

        #Passing numberOfDevices by value
        numberOfDevices = ctypes.c_uint32 ()

        #Passing DeviceList By Reference by using byref
        deviceList = DeviceInfo()

        if(listDevicesResult(numberOfDevices,ctypes.byref(deviceList)) == 1):
            print("\nSUCCESS IN GETTING DEVICE LIST\n")
            print(f'\nDEVICE NAME : {deviceList.deviceName}')
            #print("DEVICE NAME :",deviceList.deviceName)
            # print 'DEVICE NAME :',DeviceInfo.deviceName
            # print 'VENDOR ID   :',DeviceInfo.vid
        else:
            print("\nFAILED IN GETTING DEVICE LIST\n")


''' DRIVER FUNCTION '''
if __name__ == "__main__":
    intro()
    main = MainClass()
