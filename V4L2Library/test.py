from ctypes import*
import os

sharedLibrary = "/home/sushanth/Git/e-con-training/V4L2Library/libv4l2dev.so"
my_functions = CDLL(sharedLibrary)
index=1
print(my_functions.openDevice(index))
