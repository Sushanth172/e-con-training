from ctypes import*
so_file = "/home/sushanth/Git/e-con-training/V4L2Library/libv4l2dev.so"
my_functions = CDLL(so_file)
index=1
my_functions.openDevice(index)
