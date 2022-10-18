#define DFU_DETACH		0 
#define DFU_DNLOAD		1 
#define DFU_UPLOAD		2 
#define DFU_GETSTATUS		3 
#define DFU_CLRSTATUS		4 
#define DFU_GETSTATE		5 
#define DFU_ABORT       	6 
#define DFU_START		7 
#define DFU_END			8 
 
/* DFU states */ 
#define USB_REQ_DFU_DETACH	0x00 
#define USB_REQ_DFU_DNLOAD	0x01 
#define USB_REQ_DFU_UPLOAD	0x02 
#define USB_REQ_DFU_GETSTATUS	0x03 
#define USB_REQ_DFU_CLRSTATUS	0x04 
#define USB_REQ_DFU_GETSTATE	0x05 
#define USB_REQ_DFU_ABORT	0x06 
 
// --- Interface Descriptor --------------- 
 
typedef struct { 
	uint8_t bStatus; 
	uint8_t bwPollTimeout[3]; 
	uint8_t bState; 
	uint8_t iString; 
}DFU_STATUS; 
 
enum dfu_state { 
	DFU_STATE_appIDLE = 0, 
	DFU_STATE_appDETACH, 
	DFU_STATE_dfuIDLE, 
	DFU_STATE_dfuDNLOAD_SYNC, 
	DFU_STATE_dfuDNBUSY, 
	DFU_STATE_dfuDNLOAD_IDLE, 
	DFU_STATE_dfuMANIFEST_SYNC, 
	DFU_STATE_dfuMANIFEST, 
	DFU_STATE_dfuMANIFEST_WAIT_RST, 
	DFU_STATE_dfuUPLOAD_IDLE, 
	DFU_STATE_dfuERROR, 
}; 
