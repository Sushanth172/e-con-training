import os
from pyudev import Context

'''
    THIS CLASS PROVIDES METHOD TO ACCESS HID CONTROLS.
'''
class hidControl:

    '''
       THIS INIT METHOD IS CALLED WHEN THE OBJECT OF THIS CLASS IS CREATED.
    '''
    def __init__(self):
        self.hid_device_path =' '
        self.hid_handle = None


    '''
        Method Name: This method init the hid handle
        :param vid: The vendor id of the device
        :type vid: str
        :param pid: product id of the selected device
        :type pid: str
        :param device_path: device path at which the device is connected to the system
        :type device_path: str
        :return: True or False
        :rtype: bool
    '''
    def init_hid(self ,vid ,pid ,device_path):
        if self.get_hid_device_path(vid, pid):
                self.hid_handle = self.open_hid_handle()
                return True


    '''
        Method Name: open_hid_handle
        Description: This method open the hid handle of the device
        :return: True or False
        :rtype: bool
    '''
    def open_hid_handle(self):
        return os.open(self.hid_device_path, os.O_RDWR, os.O_NONBLOCK)


    '''
        Method Name: get_hid_device_path
        Description: This method get the device path of device corresponding to the vid and pid
        :param vid: The vendor id of the device
        :type vid: str
        :param pid: product id of the selected device
        :type pid: str
        :return: True or False
        :rtype: bool
    '''
    def get_hid_device_path(self, vid, pid):
        device_count = 0
        for device in Context().list_devices(subsystem='hidraw'):
            usb_device = device.find_parent('usb', 'usb_device')
            if usb_device:
                vendor_id = usb_device.get('ID_VENDOR_ID')
                product_id = usb_device.get('ID_MODEL_ID')
                if str(vendor_id) == vid and str(product_id) == pid:  # need to add [and device_count == device_number]
                    self.hid_device_path = device.device_node
                    return True
            device_count += 1
        return False


    '''
        Method Name: deinit_hid
        Description: This method closes the hid handle of the device
        :return: True or False
        :rtype: bool
    '''
    def deinit_hid(self):
            if self.hid_handle is not None:
                return os.close(self.hid_handle)
