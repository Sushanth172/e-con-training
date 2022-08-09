import ctypes

dataType = ctypes.c_char * 64
class Device(ctypes.Structure):
    _fields_ = [  ("Name",dataType),("Role",dataType) ]

def main():
    sharedLibrary = "/home/sushanth/Git/e-con-training/V4L2Library/libstructure.so"
    depthVistaLib = ctypes.CDLL(sharedLibrary)

    Result = depthVistaLib.function
    Result.argtypes = [ctypes.POINTER(Device)]

    Object = Device()

    Object.Name =  b'Sushanth'
    Object.Role =  b'Student'
    value = Object.Name.decode('utf-8')
    print(f'\nBefore Function Call (Name): {value}')
    value2 = Object.Role.decode('utf-8')
    print(f'\nBefore Function Call (Name): {value2}')

    if(Result(ctypes.byref(Object)) == 1):
        value = Object.Name.decode('utf-8')
        print(f'\nAfter Function Call (Name): {value}')

        value2 = Object.Role.decode('utf-8')
        print(f'\nAfter Function Call (Name): {value2}')


''' DRIVER FUNCTION '''
if __name__ == "__main__":
    main()

