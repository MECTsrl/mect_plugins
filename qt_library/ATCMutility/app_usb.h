#ifndef __APP_USB_H_
#define __APP_USB_H_

#ifdef __cplusplus
extern "C" {
#endif

#define APP_USB_MAX 5

/* Types Definition */
typedef struct app_usb_mount_t {
    char dev[15];
    char dev1[15];
    char mpoint[25];
} app_usb_mount_s;


/* Global Variables */
extern unsigned short app_usb_status[APP_USB_MAX + 2];
extern app_usb_mount_s app_usb[APP_USB_MAX];
extern unsigned short app_usb_feedback[2];


/* Public Functions */
short int app_usb_status_read(void);
void app_usb_init( void );
unsigned short app_usb_umount( unsigned short index ); 
unsigned short app_usb_mount( unsigned short index );
char * app_usb_mpoint_return( unsigned short index ); 

/*PLC view*/
extern int USBstatus[APP_USB_MAX + 1];
extern int USBfeedback[2];
unsigned short Usb_on(unsigned short index );
unsigned short Usb_off(unsigned short index );
char * Usb_mpoint( unsigned short index );
unsigned short Usb_copy(char *srcfile, char *dstfile, unsigned short src_index, unsigned short dst_index );
unsigned short Usb_diskcopy(char *srcfile, char *dstfile, unsigned short src_index );
unsigned short Usb_delete(char *filename, unsigned short index );
unsigned short Usb_mkdir(char *dirname, unsigned short index );
unsigned short HMItextReport(char *filename, unsigned short index );

#ifdef __cplusplus
}
#endif
#endif

