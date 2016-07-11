#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <unistd.h>
#include <dirent.h>

#include "app_logprint.h"
#include "app_usb.h"

#define SCSI_DRIVER_PATH "/proc/scsi/usb-storage"
#define USB_DEVICES_FILE "/proc/bus/usb/devices"
#define MAX_LINE_SIZE 256
#define USBDEV_COMMON "/dev/sd"
#define MNTDIR_COMMON "/local/mnt"

#define DBGUSB 0

#define DBGPOLL 0


/*Types Definition*/

/*Data structure that keep information about scsi disk gained at /proc/scsi/usb-storage/ */
typedef struct app_usb_scsiview_t {
    char vendor[40];
    char serial[40];
    char mpoint;
    short dev;
} app_usb_scsiview_s;

/*Data structure that keep information about usb tree gained at /proc/bus/usb/devices*/
typedef struct app_usb_devices_t {
    unsigned short lev;
    unsigned short port;
    unsigned short prnt;
    unsigned short cnt;
    unsigned short dev;
    unsigned short mxch;
    unsigned short kind;
    char vendor[40];
    char serial[40];
} app_usb_device_s;


/* Global Variables */
unsigned short app_usb_status[APP_USB_MAX + 2];
app_usb_mount_s app_usb[APP_USB_MAX];
unsigned short app_usb_feedback[2];

/*We need to keep previous status of scsi connected device and current status; modification
  are assessed by comparison*/
app_usb_scsiview_s app_usb_scsiview[APP_USB_MAX];
app_usb_scsiview_s app_usb_scsiview_current[APP_USB_MAX];

/*Array of available mount point since mount points are dinamically associated*/
char app_usb_available_mpoint[APP_USB_MAX];

struct copy_params {
    char *srcfile;
    char *dstfile;
    unsigned short src_index;
    unsigned short dst_index;
};

struct copy_params app_usb_copy_file_params;
pthread_t app_usb_copy_file_thread_id;
pthread_t app_usb_diskcopy_file_thread_id;

struct delete_params {
    char *filename;
    unsigned short index;
};

struct delete_params app_usb_delete_file_params;
struct delete_params app_usb_report_file_params;
pthread_t app_usb_delete_file_thread_id;
pthread_t app_usb_report_file_thread_id;


/*Local Functions*/
void *app_usb_file_copy_manager(void *param );
void *app_usb_file_diskcopy_manager(void *param );
void *app_usb_file_delete_manager(void *param );
void *app_usb_file_report_manager(void *param );

/* Public Functions*/


/**
 * Mount an usb key
 *
 * @param index  key to be mounted
 *
 *
 * @return	0 for success 1 for error
 *
 * @ingroup usb
 */

unsigned short
app_usb_mount( unsigned short index ) 
{
    char id[2]="";
    int ll;
    short rv = 1;

#if DBGUSB	
    LOG_PRINT(error_e,"Enter: %s %d %d\n",__func__, index, USBstatus[index]);
#endif	
    app_usb[index].dev[0] = '\0';
    app_usb[index].dev1[0] = '\0';

    if( index > 0 && index < APP_USB_MAX && USBstatus[index] >= 97 )
    {

        id[0] = (char)USBstatus[index];
        id[1] = '\0';

        /* Define the device name and the related mount point */

        strcat (app_usb[index].dev, USBDEV_COMMON);
        strcat (app_usb[index].dev, id);
        ll = strlen(USBDEV_COMMON)+ 1;
        app_usb[index].dev[ll] = '\0';
#if DBGUSB	
        LOG_PRINT(error_e, "%s %s %s\n", app_usb[index].dev,  app_usb[index].dev1, app_usb[index].mpoint );
#endif	
        strcat (app_usb[index].mpoint, MNTDIR_COMMON);
        strcat (app_usb[index].mpoint, id);
        ll = strlen(MNTDIR_COMMON)+ 1;
        app_usb[index].mpoint[ll] = '\0';
#if DBGUSB	
        LOG_PRINT(error_e, "%s %s %s\n", app_usb[index].dev,  app_usb[index].dev1, app_usb[index].mpoint );
#endif		
        strcat(app_usb[index].dev1,app_usb[index].dev);
        id[0] = '1';
        strcat (app_usb[index].dev1, id);
        ll = strlen(app_usb[index].dev)+ 1;
        app_usb[index].dev1[ll] = '\0';

        /* FIXME: find a better way to check the usb dev */
        {
            FILE * fp = fopen(app_usb[index].dev1, "r");
            if (fp)
            {
                fclose(fp);
            }
            else
            {
                LOG_PRINT(error_e, "Cannot mount '%s'\n", app_usb[index].dev1);
                strcpy(app_usb[index].dev1, app_usb[index].dev);
                FILE * fp = fopen(app_usb[index].dev1, "r");
                if (fp)
                {
                    fclose(fp);
                }
                else
                {
                    LOG_PRINT(error_e, "Cannot mount '%s'\n", app_usb[index].dev1);
                    return rv;
                }
            }
        }

#if DBGUSB	
        LOG_PRINT(error_e, "%s %s %s\n", app_usb[index].dev,  app_usb[index].dev1, app_usb[index].mpoint );
#endif	
        /* test the mount point existence if failed create mpoint */
        if ( access(app_usb[index].mpoint, F_OK )!= 0) {

            if( mkdir( app_usb[index].mpoint, S_IRWXU | S_IRWXG | S_IRWXO )!= 0 ){
                LOG_PRINT(error_e, "mkdir %s : %s\n", __func__, strerror(errno));
                return rv;
            }

        }

        /* Trying to mount the device, mount returns 0 on success and -1 on failure*/

        do
        {
#if DBGUSB
            LOG_PRINT(error_e, "mounting %s -> %s [%d]\n", app_usb[index].dev1, app_usb[index].mpoint, errno);
#endif
            rv = mount (app_usb[index].dev1, app_usb[index].mpoint, "vfat", 0, (void *)NULL );
            if (rv == -1 && (errno == ENOENT || errno == EBUSY))
            {
                sync();
                sleep (1);
            }
        }
        while (errno == ENOENT && errno != EBUSY);
        /* FIXME: for some reason mount fail, errno is EBUSY but the USB drive is mounted correctly */
        if (errno == EBUSY)
        {
            rv = 0;
        }
        if (rv == -1){
            LOG_PRINT(error_e, "mount %s -> %s : %s [%d]\n", app_usb[index].dev1, app_usb[index].mpoint, strerror(errno), errno);
            rv = 1;
        }
        if (rv){
            do
            {
#if DBGUSB
                LOG_PRINT(error_e, "mounting %s -> %s [%d]\n", app_usb[index].dev, app_usb[index].mpoint, errno);
#endif
                rv = mount (app_usb[index].dev, app_usb[index].mpoint, "vfat", 0, (void *)NULL );
                if (rv == -1 && (errno == ENOENT || errno == EBUSY))
                {
                    sync();
                    sleep (1);
                }
            }
            while (errno == ENOENT && errno != EBUSY);
            /* FIXME: for some reason mount fail, errno is EBUSY but the USB drive is mounted correctly */
            if (errno == EBUSY)
            {
                rv = 0;
            }
            if (rv == -1){
                LOG_PRINT(error_e, "mount2 %s -> %s : %s [%d]\n", app_usb[index].dev, app_usb[index].mpoint, strerror(errno), errno);
                rv = 1;
            }

        }

        return rv;
    }

    LOG_PRINT(error_e, "index %d APP_USB_MAX %d USBstatus[%d] %d vs 97\n", index, APP_USB_MAX, index, USBstatus[index]);
    return rv;
}


/**
 * Unmount an usb key
 *
 * @param index  key to be mounted
 *
 * @return	0 for success 1 for error
 *
 * @ingroup usb
 */

unsigned short
app_usb_umount( unsigned short index ) 
{
    char id[2]="";
    int ll;

    unsigned short rv = 1;

#if DBGUSB	
    LOG_PRINT(error_e,"Enter: %s %d %d\n",__func__, index, USBstatus[index]);
#endif	
    if( index > 0 && index < APP_USB_MAX && USBstatus[index] >= 97 ){


        id[0] = (char)USBstatus[index];
        id[1] = '\0';

        strcat (app_usb[index].mpoint, MNTDIR_COMMON);
        strcat (app_usb[index].mpoint, id);
        ll = strlen(MNTDIR_COMMON)+ 1;
        app_usb[index].mpoint[ll] = '\0';

        rv = umount2(app_usb[index].mpoint, MNT_FORCE);

        if(!rv){
            /* Clean usb info */
            strcpy(app_usb[index].dev , "");
            strcpy(app_usb[index].dev1, "");
            strcpy(app_usb[index].mpoint, "");
            return rv;
        }
        strcpy(app_usb[index].dev , "");
        strcpy(app_usb[index].dev1, "");
        strcpy(app_usb[index].mpoint, "");
#if DBGUSB	
        LOG_PRINT(error_e,"%s dev %s, dev1 %s, mpoint %s\n",__func__, app_usb[index].dev, app_usb[index].dev1, app_usb[index].mpoint);
#endif				
        return 1;

    }
    return rv;
}

/**
 * Create a directory on a usb key
 *
 * @param char *dirname directory to be created
 * @param unsigned short index  index representing the usbkey on which the directory will exist
 *
 * @return	0 for success 1 for error 2 directory already exist
 *
 * @ingroup usb
 */

static unsigned short
app_usb_dir_create( char *dirname, unsigned short index )
{
    char *full_dirname;
    char *l;

    if (!USBfeedback[0]) {

        full_dirname = (char *)calloc((strlen(dirname)+ 1 + strlen(app_usb[index].mpoint) + 1 + 2 ), sizeof (char));

        l = dirname;
        if (*l == '.' && ( *(l+1) == '/' || *(l+1) == '\\' ))
            l = dirname + 2;

        strcpy(full_dirname, app_usb[index].mpoint);
        strcat(full_dirname, "/");
        strcat(full_dirname,  l );
        for(l = full_dirname; *l != '\0'; l++)
            if (*l == '\\')
                *l = '/';
        if ( access(full_dirname, F_OK )!= 0) {
            if( mkdir( full_dirname, S_IRWXU | S_IRWXG | S_IRWXO )!= 0 ){
                free(full_dirname);
                return 1;
            } else {
                free(full_dirname);
                return 0;
            }
        } else {
            free(full_dirname);
            return 2;
        }


    } else
        return 1;
}




/**
 * Copy either a file or a directory with all its content between usb keys
 *
 * @param char *filename file o directory to be copied
 * @param unisgned short src_index   index in the USBstatus array  representing the source usbkey for filename
 * @param unisgned short dst_index   index in the USBstatus array  representing the destination usbkey for filename
 *
 * @return	0 for success,  1 for error: resource already busy cannot start another usb operation
 *
 * @ingroup usb
 */
static unsigned short
app_usb_file_copy(char *srcfile, char *dstfile, unsigned short src_index, unsigned short dst_index )
{

#if DBGUSB
    LOG_PRINT(error_e,"Enter: %s\n", __func__);
#endif

    if (!USBfeedback[0]) {
        USBfeedback[0] = app_usb_feedback[0] = 1; /*Set to one flag about operation on usb running -- usb locked*/
        USBfeedback[1] = app_usb_feedback[1] = 0; /*Reset error code for current running operation on usb*/

        app_usb_copy_file_params.srcfile = srcfile;
        app_usb_copy_file_params.dstfile = dstfile;
        LOG_PRINT(verbose_e,"src %s dest %s\n",app_usb_copy_file_params.srcfile, app_usb_copy_file_params.dstfile);
        app_usb_copy_file_params.src_index = src_index;
        app_usb_copy_file_params.dst_index = dst_index;

        pthread_create(&app_usb_copy_file_thread_id, NULL, &app_usb_file_copy_manager, &app_usb_copy_file_params);

        return 0;
    } else
        return 1;

}


/**
 * Copy either a file or a directory with all its content between usb keys -- thread manager
 * Status of execution handling is stored in USBfeedback[1]
 *
 * 				0 for success,  > 0 for error :
 *				1- cannot mount/umount usb key,
 *				2- source file does not exist,
 *				3- there's no enough space on destination usbkey to perform the copy
 *				4- copy failed
 * @param		Void *param - structure containg all the info required to perform the copy.
 * @return		NULL
 *
 * @ingroup usb
 */
void *
app_usb_file_copy_manager(void *param )
{
    struct copy_params *p = (struct copy_params *)param;
#if DBGUSB
    LOG_PRINT(error_e,"Enter: %s\n", __func__);
#endif

    unsigned short rv = 1;
    //FILE *fp;
    // int dst_free_space = 0;
    int flash_root = 0;
    char dst_mpoint[FILENAME_MAX]="";
    char src_mpoint[FILENAME_MAX]="";
    char *c;
    char *cmd;
    char *dst_path;
    char *src_path;
    char *l;
    struct stat filename_stat;

    LOG_PRINT(error_e, "####################### src %s dest %s\n",p->srcfile, p->dstfile);

    /* Mount the usb key involved in file copying */
    if ( p->dst_index == 0 )
    {
#if DBGUSB	
        LOG_PRINT(error_e,"Not allowed value for dst_index\n");
#endif	
        app_usb_feedback[1]= 1;
        app_usb_feedback[0]= 0;
        return NULL;
    }
    if ( p->src_index != 0 &&  p->src_index != p->dst_index ){
        if ( app_usb_mount( p->dst_index ) || app_usb_mount( p->src_index ) ){
            app_usb_feedback[1]= 1;
            app_usb_feedback[0]= 0;
#if DBGUSB	
            LOG_PRINT(error_e,"failed to mount  one of the two key\n");
#endif			
            return NULL;
        }
    }
    else if ( p->src_index == 0 )
    {
        if ( app_usb_mount( p->dst_index ) ){
            app_usb_feedback[1]= 1;
            app_usb_feedback[0]= 0;
#if DBGUSB	
            LOG_PRINT(error_e,"FAILED dst_mpoint: %s\n", dst_mpoint);
#endif			
            return NULL;
        }
        flash_root = 1;
    }
    else
    {
        if ( app_usb_mount( p->src_index ) ){
            app_usb_feedback[1]= 1;
            app_usb_feedback[0]= 0;
#if DBGUSB	
            LOG_PRINT(error_e,"dst_mpoint: %s\n", dst_mpoint);
#endif			
            return NULL;
        }
    }

    /* Check if destination has enough space to host filename*/
    strcpy ( dst_mpoint, app_usb_mpoint_return( p->dst_index ) );
#if DBGUSB	
    LOG_PRINT(error_e,"dst_mpoint: %s\n", dst_mpoint);
#endif	

#if 0	
    cmd = strdup("df -k  | grep %s | awk '{print $4}'");
    assert(cmd!=NULL);
    c = (char *)calloc((strlen(cmd) + strlen(dst_mpoint) +1 ), sizeof(char));
    sprintf(c, cmd, dst_mpoint);
    fp = popen(c, "r");
    assert(fp!=NULL);
    if (fscanf(fp, "%u", &dst_free_space) != EOF) {

        pclose(fp);
        free(cmd);
        free(c);
    }
#endif	
    if (!flash_root)
    {
        strcpy ( src_mpoint, app_usb_mpoint_return( p->src_index ) );
    }
    else
    {
        strcpy ( src_mpoint, "" );
    }
#if DBGUSB	
    LOG_PRINT(error_e,"src_mpoint: %s\n", src_mpoint);
#endif	
    src_path = (char *)calloc((strlen(src_mpoint) + strlen(p->srcfile) + 2 ), sizeof(char));
#if 0
    strcat(src_path, src_mpoint);
    l = p->srcfile;
    if (*l == '.' && ( *(l+1) == '/' || *(l+1) == '\\' ))
        l = p->srcfile + 2;
    if (!flash_root)
    {
        strcat(src_path, "/");
    }
    strcat(src_path,  l );
    for(l = src_path; *l != '\0'; l++)
        if (*l == '\\')
            *l = '/';
#else
    sprintf(src_path, "%s/%s", src_mpoint, p->srcfile);
#endif
#if DBGUSB	
    LOG_PRINT(error_e,"file to copy: %s -> %s\n", p->srcfile, src_path);
#endif	
    if ( (stat(src_path, &filename_stat)) == -1 ){
        LOG_PRINT(error_e, "ERRORE: %s\n", strerror(errno));
        app_usb_feedback[1]= 2;
        app_usb_feedback[0]= 0;
        goto unmount;
#if 0
    } else if (filename_stat.st_size >= (dst_free_space * 1024) ){
#if DBGUSB	
        LOG_PRINT(error_e,"file size: %d, free space = %d\n",filename_stat.st_size, dst_free_space );
#endif		
        app_usb_feedback[1]= 3;
        app_usb_feedback[0]= 0;
        return NULL;
#endif		

    } else { /* There is space to copy the file to the destination key*/

        /*build destination path*/
        LOG_PRINT(error_e,"before smanazz : %s\n", p->dstfile);
        dst_path = (char *)calloc((strlen(dst_mpoint) + strlen(p->dstfile) + 2 ), sizeof(char));
        strcat(dst_path, dst_mpoint);
        l = p->dstfile;
        if (*l == '.' && ( *(l+1) == '/' || *(l+1) == '\\' ))
            l = p->dstfile + 2;
        strcat(dst_path, "/");
        strcat(dst_path,  l );
        for(l = dst_path; *l != '\0'; l++)
            if (*l == '\\')
                *l = '/';
#if DBGUSB					
        LOG_PRINT(error_e,"destination: %s -> %s\n", p->dstfile, dst_path);
#endif			
        /*perform copy*/
        cmd = strdup("cp -a %s %s");
        c = (char *)calloc((strlen(cmd) + strlen(src_path) +1 + strlen(dst_path) +1 ), sizeof(char));
        sprintf(c, cmd, src_path, dst_path);
        LOG_PRINT(verbose_e,"'%s' - '%s' - '%s'\n", src_path, dst_path, c);
#if DBGUSB		
        LOG_PRINT(error_e,"copy command: %s\n", c);
#endif			
        rv = system(c);
        free(cmd);
        free(c);
        free(src_path);
        free(dst_path);
        if(rv != 0){
            app_usb_feedback[1]= 3; //app_usb_feedback[1]= 4;
            app_usb_feedback[0]= 0;
            goto unmount;
        }
    }

    app_usb_feedback[1]= 0;
    app_usb_feedback[0]= 0;

unmount:
    /* Unmount usb devices */
    if ( p->src_index != 0 &&  p->src_index != p->dst_index ){
        if ( app_usb_umount( p->src_index ) || app_usb_umount( p->dst_index ) ){
            app_usb_feedback[1]= 1;
            app_usb_feedback[0]= 0;
            return NULL;
        }
    }
    else if ( p->src_index == 0)
    {
        if ( app_usb_umount( p->dst_index ) ){
            app_usb_feedback[1]= 1;
            app_usb_feedback[0]= 0;
            return NULL;
        }
    }
    else
    {
        if ( app_usb_umount( p->src_index ) ){
            app_usb_feedback[1]= 1;
            app_usb_feedback[0]= 0;
#if DBGUSB	
            LOG_PRINT(error_e,"dst_mpoint: %s\n", dst_mpoint);
#endif			
            return NULL;
        }
    }

    return NULL;

}	


/**
 * Copy either a file or a directory with all its content from a usb key to internal flash
 *
 * @param char *srcfile file o directory to be copied
 * @param char *dstfile filename o directory name  for destination
 * @param unisgned short src_index   index in the USBstatus array  representing the source usbkey for filename
 *
 * @return	0 for success,  1 for error: resource already busy cannot start another usb operation
 *
 * @ingroup usb
 */
static unsigned short
app_usb_file_diskcopy(char *srcfile, char *dstfile, unsigned short src_index )
{

    if (!USBfeedback[0]) {
        USBfeedback[0] = app_usb_feedback[0] = 1; /*Set to one flag about operation on usb running -- usb locked*/
        USBfeedback[1] = app_usb_feedback[1] = 0; /*Reset error code for current running operation on usb*/

        app_usb_copy_file_params.srcfile = strdup(srcfile);
        app_usb_copy_file_params.dstfile = strdup(dstfile);
        app_usb_copy_file_params.src_index = src_index;
        app_usb_copy_file_params.dst_index = 0;

        pthread_create(&app_usb_diskcopy_file_thread_id, NULL, &app_usb_file_diskcopy_manager, &app_usb_copy_file_params);

        return 0;
    } else
        return 1;

}


/**
 * Copy either a file or a directory with all its content between usb keys -- thread manager
 * Status of execution handling is stored in USBfeedback[1]
 *
 * 				0 for success,  > 0 for error :
 *				1- cannot mount/umount usb key,
 *				2- source file does not exist,
 *				3- there's no enough space on destination usbkey to perform the copy
 *				4- copy failed
 * @param		Void *param - structure containg all the info required to perform the copy.
 * @return		NULL
 *
 * @ingroup usb
 */
void *
app_usb_file_diskcopy_manager(void *param )
{
#define DST_MPOINT "/local/etc/sysconfig"

    struct copy_params *p = (struct copy_params *)param;

    unsigned short rv = 1;
    FILE *fp;
    int dst_free_space = 0;
    char dst_mpoint[22]="";
    char src_mpoint[22]="";
    char *c;
    char *cmd;
    char *dst_path;
    char *src_path;
    char *l;
    struct stat filename_stat;

    /* Mount the usb key involved in file copying */
    if ( app_usb_mount( p->src_index ) ){
        app_usb_feedback[1]= 1;
        app_usb_feedback[0]= 0;
        return NULL;
    }


    /* Check if destination has enough space to host filename*/
    strcpy ( dst_mpoint, DST_MPOINT );
#if DBGUSB	
    LOG_PRINT(error_e,"dst_mpoint: %s\n", dst_mpoint);
#endif	
    cmd = strdup("df -k %s | awk 'NR==2 {print $4}'");
#if DBGUSB	
    assert(cmd!=NULL);
#else
    app_usb_feedback[1]= 1;
    app_usb_feedback[0]= 0;
    return NULL;
#endif	
    c = (char *)calloc((strlen(cmd) + strlen(dst_mpoint) +1 ), sizeof(char));
    sprintf(c, cmd, dst_mpoint);
#if DBGUSB	
    LOG_PRINT(error_e,"command %s\n",c );
#endif	
    fp = popen(c, "r");

#if DBGUSB	
    assert(fp!=NULL);
#else
    app_usb_feedback[1]= 1;
    app_usb_feedback[0]= 0;
    return NULL;
#endif	
    if (fscanf(fp, "%u", &dst_free_space) != EOF) {

        pclose(fp);
        free(cmd);
        free(c);
    }
#if DBGUSB	
    LOG_PRINT(error_e," free space = %d\n", dst_free_space );
#endif		
    strcpy ( src_mpoint, app_usb_mpoint_return( p->src_index ) );
#if DBGUSB	
    LOG_PRINT(error_e,"src_mpoint: %s\n", src_mpoint);
#endif	
    src_path = (char *)calloc((strlen(src_mpoint) + strlen(p->srcfile) + 2 ), sizeof(char));
    strcat(src_path, src_mpoint);
    l = p->srcfile;
    if (*l == '.' && ( *(l+1) == '/' || *(l+1) == '\\' ))
        l = p->srcfile + 2;
    strcat(src_path, "/");
    strcat(src_path,  l );
    for(l = src_path; *l != '\0'; l++)
        if (*l == '\\')
            *l = '/';

#if DBGUSB	
    LOG_PRINT(error_e,"file to copy: %s\n", src_path);
#endif	
    if ( (stat(src_path, &filename_stat)) == -1 ){
        LOG_PRINT(error_e, "ERRORE: %s\n", strerror(errno));
        app_usb_feedback[1]= 2;
        app_usb_feedback[0]= 0;
        goto unmount;
    } else if (filename_stat.st_size >= (dst_free_space * 1024) ){
#if DBGUSB	
        LOG_PRINT(error_e,"file size: %d, free space = %d\n",(int)(filename_stat.st_size), dst_free_space );
#endif		
        app_usb_feedback[1]= 3;
        app_usb_feedback[0]= 0;
        goto unmount;

    } else { /* There is space to copy the file to the destination key*/

        /*build destination path*/
        dst_path = (char *)calloc((strlen(dst_mpoint) + strlen(p->dstfile) + 2 ), sizeof(char));
        strcat(dst_path, dst_mpoint);
        l = p->dstfile;
        if (*l == '.' && ( *(l+1) == '/' || *(l+1) == '\\' ))
            l = p->dstfile + 2;
        strcat(dst_path, "/");
        strcat(dst_path,  l );
        for(l = dst_path; *l != '\0'; l++)
            if (*l == '\\')
                *l = '/';
#if DBGUSB					
        LOG_PRINT(error_e,"destination: %s\n", dst_path);
#endif			
        /*perform copy*/
        cmd = strdup("cp -a %s %s");
        c = (char *)calloc((strlen(cmd) + strlen(src_path) +1 + strlen(dst_path) +1 ), sizeof(char));
        sprintf(c, cmd, src_path, dst_path);
        LOG_PRINT(verbose_e,"'%s' - '%s' - '%s'\n", src_path, dst_path, c);
#if DBGUSB		
        LOG_PRINT(error_e,"copy command: %s\n", c);
#endif			
        rv = system(c);
        free(cmd);
        free(c);
        free(src_path);
        free(dst_path);
        if(rv != 0){
            app_usb_feedback[1]= 4;
            app_usb_feedback[0]= 0;
            goto unmount;
        }
    }

    /* Unmount usb devices */

    app_usb_feedback[1]= 0;
    app_usb_feedback[0]= 0;

unmount:
    if ( app_usb_umount( p->src_index ) ){
        app_usb_feedback[1]= 1;
        app_usb_feedback[0]= 0;
        return NULL;
    }

    free (p->srcfile);
    free (p->dstfile);

    return NULL;

#undef DST_MPOINT	
}	




/**
 * Delete a file from a usb key
 *
 * @param char *filename file o directory to be deleted
 * @param unsigned short src_index   index representing the usbkey on which the file is stored
 *
 * @return	0 for success 1 for error
 *
 * @ingroup usb
 */

static unsigned short
app_usb_file_delete( char *filename, unsigned short index )
{

    if (!USBfeedback[0]) {
        USBfeedback[0] = app_usb_feedback[0] = 1; /*Set to one flag about operation on usb running -- usb locked*/
        USBfeedback[1] = app_usb_feedback[1] = 0; /*Reset error code for current running operation on usb*/

        app_usb_delete_file_params.filename = filename;
        app_usb_delete_file_params.index = index;

        pthread_create(&app_usb_delete_file_thread_id, NULL, &app_usb_file_delete_manager, &app_usb_delete_file_params);

        return 0;
    } else
        return 1;
}



/**
 * Delete a file from a usb key -- thread manager
 * Status of execution handling is stored in USBfeedback[1]: 0 for success,   1 for error:
 *
 * @param	void *param -- parameters required to handle delete operation
 * @return	NULL
 *
 * @ingroup usb
 */

void *
app_usb_file_delete_manager( void *param )
{
    struct delete_params *p = (struct delete_params *)param;

    unsigned short rv = 1;
    char mpoint[22]="";
    char *c;
    char *cmd;
    char *path;
    char *l;

    if ( app_usb_mount( p->index ) ){
        app_usb_feedback[1]= 1;
        app_usb_feedback[0]= 0;
        return NULL;

    }

    strcpy ( mpoint, app_usb_mpoint_return( p->index ) );
    path = (char *)calloc((strlen(mpoint) + strlen(p->filename) + 2 ), sizeof(char));
    strcat(path, mpoint);
    l = p->filename;
    if (*l == '.' && ( *(l+1) == '/' || *(l+1) == '\\' ))
        l = p->filename + 2;
    strcat(path, "/");
    strcat(path,  l );
    for(l = path; *l != '\0'; l++)
        if (*l == '\\')
            *l = '/';

    /*perform delete*/
    cmd = strdup("rm -rf %s");
    c = (char *)calloc((strlen(cmd) + strlen(path) +1 ), sizeof(char));
    sprintf(c, cmd, path);
    rv = system(c);
    free(cmd);
    free(c);
    free(path);
    if(rv != 0){
        app_usb_feedback[1]= 1;
        app_usb_feedback[0]= 0;

    }
    else
    {
        app_usb_feedback[1]= 0;
        app_usb_feedback[0]= 0;
    }
    /*Unmount the key*/
    if ( app_usb_umount( p->index ) ){
        app_usb_feedback[1]= 1;
        app_usb_feedback[0]= 0;
        return NULL;
    }

    return NULL;

}

/**
 * Create a  report file on a usb key from /local/hmitext directory content
 *
 * @param char *filename file to be creates
 * @param unsigned short src_index   index representing the usbkey on which the file is stored
 *
 * @return	0 for success 1 for error
 *
 * @ingroup usb
 */

static unsigned short
app_usb_file_report( char *filename, unsigned short index )
{

    if (!USBfeedback[0]) {
        USBfeedback[0] = app_usb_feedback[0] = 1; /*Set to one flag about operation on usb running -- usb locked*/
        USBfeedback[1] = app_usb_feedback[1] = 0; /*Reset error code for current running operation on usb*/

        app_usb_report_file_params.filename = filename;
        app_usb_report_file_params.index = index;

        pthread_create(&app_usb_report_file_thread_id, NULL, &app_usb_file_report_manager, &app_usb_report_file_params);

        return 0;
    } else
        return 1;
}


/**
 * Create a file report onusb key -- thread manager
 * Status of execution handling is stored in USBfeedback[1]: 0 for success,   1 for error:
 *
 * @param	void *param -- parameters required to handle delete operation
 * @return	NULL
 *
 * @ingroup usb
 */

void *
app_usb_file_report_manager( void *param )
{
    struct delete_params *p = (struct delete_params *)param;

    unsigned short rv = 1;
    char mpoint[22]="";
    char *c;
    char *cmd;
    char *path;
    char *l;

    if ( app_usb_mount( p->index ) ){
        app_usb_feedback[1]= 1;
        app_usb_feedback[0]= 0;
        return NULL;

    }

    strcpy ( mpoint, app_usb_mpoint_return( p->index ) );
    path = (char *)calloc((strlen(mpoint) + strlen(p->filename) + 2 ), sizeof(char));
    strcat(path, mpoint);
    l = p->filename;
    if (*l == '.' && ( *(l+1) == '/' || *(l+1) == '\\' ))
        l = p->filename + 2;
    strcat(path, "/");
    strcat(path,  l );
    for(l = path; *l != '\0'; l++)
        if (*l == '\\')
            *l = '/';

    /*perform report*/
#if DEBUG
    LOG_PRINT(error_e," %s path %s \n", __func__, path);
#endif	
    cmd = strdup("ls /local/hmitext/* | xargs sort > %s");
    c = (char *)calloc((strlen(cmd) + strlen(path) +1 ), sizeof(char));
    sprintf(c, cmd, path);
#if DEBUG
    LOG_PRINT(error_e," %s command %s \n", __func__, c);
#endif	
    rv = system(c);
    free(cmd);
    free(c);
    free(path);
    if(rv != 0){
        app_usb_feedback[1]= 1;
        app_usb_feedback[0]= 0;
    }
    else
    {
        app_usb_feedback[1]= 0;
        app_usb_feedback[0]= 0;
    }
    /*Unmount the key*/
    if ( app_usb_umount( p->index ) ){
        app_usb_feedback[1]= 1;
        app_usb_feedback[0]= 0;
        return NULL;
    }

    return NULL;

}



/**
 * Retrieve the mountpoint for the given index key
 *
 * @param 	index key whose mount point needs to be retrieved.
 *
 * @return	char pointer to the mpoint.
 *
 * @ingroup usb
 */

char *
app_usb_mpoint_return( unsigned short index ){

    return app_usb[index].mpoint;
}

/**
 * Init Data structure that contains usb keys mapping device and mountpoints, scsi info and usb operation feedback.
 *
 * @param 	void
 *
 * @return	void
 *
 * @ingroup usb
 */

void
app_usb_init( void )
{
    int i;

    for (i = 0; i < APP_USB_MAX; i++) {
        strcpy(app_usb[i].dev, "");
        strcpy(app_usb[i].dev1, "");
        strcpy(app_usb[i].mpoint, "");


        /*Reset scsi_view data structure*/
        if (i < APP_USB_MAX ){
            strcpy(app_usb_scsiview[i].vendor, "");
            strcpy(app_usb_scsiview[i].serial, "");
            app_usb_scsiview[i].dev = -1;
            strcpy(app_usb_scsiview_current[i].vendor, "");
            strcpy(app_usb_scsiview_current[i].serial, "");
            app_usb_scsiview_current[i].dev = -1;
            app_usb_available_mpoint[i] = 1;
        }
        if((i == 0) || (i == 1))
            app_usb_feedback[i] = 0;
    }
#if DBGPOLL
    LOG_PRINT(error_e, "Usb Init\n");
    for( i= 0; i < APP_USB_MAX-1; i++)
        LOG_PRINT(error_e, "Previous  dev %d, mpoint %d --- Current  dev %d, mpoint %d\n", app_usb_scsiview[i].dev,app_usb_scsiview[i].mpoint,app_usb_scsiview_current[i].dev, app_usb_scsiview_current[i].mpoint);
#endif  	

}


/**
 * Usb system manager thread
 *
 * current implementation of this function cannot recover the correct mount point in case the
 * application has been stopped but the system has not been rebooted
 *
 *
 * @param 	void * dummy
 *
 * @return	short int 0 a change successfully managed, -1 for error, 1 for status quo
 *
 * @error	USBstatus[APP_USB_MAX+1] = 10 - Detected another multiport hub, only one supported
 * @error	USBstatus[APP_USB_MAX+1] = 20 - Parsing error
 * @error	USBstatus[APP_USB_MAX+1] = 30 - Cannot get device info for usb subsystem
 *
 * @ingroup usb
 */

short int
app_usb_status_read(void)
{

    FILE *fp;
    FILE *fs;
    char line[MAX_LINE_SIZE + 1]="";
    app_usb_device_s devinfo;
    int i;
    unsigned count;
    int hubkey = 0;
    int hubkey_bkp = 0;
    int scsi = 0;
    char scsi_file[30] ="";
    char scsi_id[7];
    int j;
    int k;
    char *l;
    unsigned ll = 0;
    unsigned ln = 0;
    unsigned len = 0;
    unsigned num = 0;
    long current;

    /* Structures for run time check status; we know that a new usb device has been connected
       since the modification time of the devices file changes*/

    struct stat devices_stat;
    static time_t old = 0;

    if ( (stat(USB_DEVICES_FILE, &devices_stat)) == -1 ){
        LOG_PRINT(error_e, "ERRORE: %s\n", strerror(errno));
        return -1;
    }
    else {
        if (old == devices_stat.st_mtime)
            return 1;
        else {

            old = devices_stat.st_mtime;

#if DBGPOLL		
            LOG_PRINT(error_e,"Variazione su USB\n");
#endif		
            sleep(1); /*checked on an light loaded system that it's the min time to get scsi info ready*/
            /* Init the SCSI subsystem  current information*/

            /* Check if any scsi device has been plugged, if not reset app_usb_scsiview data structure for
               vendor and serial number */
            if ( access(SCSI_DRIVER_PATH, F_OK) == -1 ) {
                for (i = 0; i < APP_USB_MAX; i++) {
                    strcpy(app_usb_scsiview[i].vendor, "");
                    strcpy(app_usb_scsiview[i].serial, "");
                    app_usb_scsiview[i].dev = -1;
                    app_usb_available_mpoint[i] = 1;
                }
            }
            else { /*there is some scsi device attached*/

                count = 0;
                scsi = 0;

                /* count the number of file into the directory /proc/scsi/usb-storage
                 * not use system or popen cause are based on fork wich use a lot of memory
                 */
                struct dirent **filelist = {0};
                int i = 0;
                int filesnb = -1;

                filesnb = scandir(SCSI_DRIVER_PATH, &filelist, 0, alphasort);

                if(filesnb < 0) {
                    LOG_PRINT(error_e,"Cannot open '%s' [%s]\n", SCSI_DRIVER_PATH, strerror(errno));
                    return -1;
                }
                else
                {
                    /* skip the files '.' and '..'*/
                    num = filesnb - 2;
                    while (filesnb > 0)
                    {
                        filesnb--;
                        free(filelist[filesnb]);
                    }
                    free(filelist);
                }
#if DBGPOLL			
                LOG_PRINT(error_e,"Current number of attached devices: %d\n", num);
#endif				
                /*We loop untill we find the number of attached device "num"*/
                while( count < num )
                {
                    len = sprintf(scsi_id, "%d",scsi);
                    scsi_id[len +1] = '\0';
                    scsi_file[0]= '\0';
                    strcat(scsi_file, SCSI_DRIVER_PATH);
                    strcat(scsi_file, "/");
                    strcat(scsi_file, scsi_id);
#if DBGPOLL			
                    LOG_PRINT(error_e,"Current scsi file: %s\n", scsi_file);
#endif		
                    if((fs = fopen(scsi_file, "r")) != NULL){
                        *line = '\0';
                        for (ln = 1; fgets(line, MAX_LINE_SIZE, fs) == line && ln < 5; ln++) {
#if DBGPOLL			   
                            LOG_PRINT(error_e, "scsi line : %s", line);
#endif				
                            if (ln == 2 || ln == 4){
                                ll = strlen(line);
                                if(line[ll - 1] == '\n')
                                    line[ll - 1] = '\0';
                                l=strchr(line, ':');
                                for(l= l+2, j=0; *l != '\0' && !( *l==' ' && *(l+1)==' ') ; l++, j++)
                                    if(ln == 2)
                                        app_usb_scsiview_current[count].vendor[j] = *l;
                                    else /*ln == 4*/
                                        app_usb_scsiview_current[count].serial[j] = *l;
                                if(ln == 2)
                                    app_usb_scsiview_current[count].vendor[j] = '\0';
                                else
                                    app_usb_scsiview_current[count].serial[j] = '\0';
                            }

                        }
                        fclose(fs);
#if DBGPOLL			   
                        LOG_PRINT(error_e,"scsi_vendor= %s, scsi_serial= %s\n", app_usb_scsiview_current[count].vendor, app_usb_scsiview_current[count].serial);
#endif			   			

                        app_usb_scsiview_current[count].dev = scsi;
                        count++;
                        scsi++;

                    } else
                        /*Simply go on with check and look for another file*/
                        scsi++;

                }/*end while*/

#if DBGPOLL
                for( i= 0; i < APP_USB_MAX-1; i++)
                    LOG_PRINT(error_e, "Previous  dev %d, mpoint %d --- Current  dev %d, mpoint %d\n", app_usb_scsiview[i].dev,app_usb_scsiview[i].mpoint,app_usb_scsiview_current[i].dev, app_usb_scsiview_current[i].mpoint);
#endif 	
                /*Compare previous scsi situation with current. Each time there is an
                  already mounted device we delete it from the current array, we keep it in
                  the "previous" array and if necessary we move it in the first free position
                  of the "previous array"
                  Previuos and Current array are built in ascending order. A dev value of -1
                  means a free location in the array. Location subsequent to a free location
                  are free as well*/

                for (i = 0; app_usb_scsiview[i].dev != -1 && i < APP_USB_MAX-1; i++){
                    for(j = 0; j < APP_USB_MAX-1; j++){
                        if (app_usb_scsiview_current[j].dev != -1){
                            if(app_usb_scsiview[i].dev == app_usb_scsiview_current[j].dev){
                                app_usb_scsiview_current[j].dev = -1;
                                strcpy(app_usb_scsiview_current[j].vendor, "\0");
                                strcpy(app_usb_scsiview_current[j].serial, "\0");
#if DBGPOLL
                                LOG_PRINT(error_e,"Device %d was already mounted at %d index %d\n",app_usb_scsiview[i].dev, app_usb_scsiview[i].mpoint, i );
#endif
                                for(k = 0; k < i; k++)
                                    if(app_usb_scsiview[k].dev != -1)
                                        continue;
                                    else { /* move the data that remain unchanged in the first available location
                                              in order to keep data structures ordered !!!!*/
                                        strcpy(app_usb_scsiview[k].vendor, app_usb_scsiview[i].vendor);
                                        strcpy(app_usb_scsiview[k].serial, app_usb_scsiview[i].serial);
                                        app_usb_scsiview[k].dev = app_usb_scsiview[i].dev;
                                        app_usb_scsiview[k].mpoint = app_usb_scsiview[i].mpoint;
                                        strcpy(app_usb_scsiview[i].vendor, "\0");
                                        strcpy(app_usb_scsiview[i].serial, "\0");
                                        app_usb_scsiview[i].dev = -1;
#if DBGPOLL
                                        LOG_PRINT(error_e,"Moved Device %d mounted at %d index %d\n",app_usb_scsiview[i].dev, app_usb_scsiview[i].mpoint, i );
#endif
                                        break;
                                    }
                                break;

                            } else if (app_usb_scsiview[i].dev > app_usb_scsiview_current[j].dev){
                                /* continue to search...*/
#if DBGPOLL
                                LOG_PRINT(error_e, "Continue to search...\n");
#endif
                                continue;

                            }

                            else { /*device has been unplugged, free the mount point */
#if DBGPOLL
                                LOG_PRINT(error_e,"Device Unplugged %d mounted at %d free mpoint index %d\n",app_usb_scsiview[i].dev, app_usb_scsiview[i].mpoint, app_usb_scsiview[i].mpoint - 'a' );
#endif						       
                                app_usb_scsiview[i].dev = -1;
                                strcpy(app_usb_scsiview[i].vendor, "\0");
                                strcpy(app_usb_scsiview[i].serial, "\0");
                                app_usb_available_mpoint[app_usb_scsiview[i].mpoint - 'a' ] = 1;

                                break;
                            }
                        } else if (app_usb_scsiview_current[j].dev == -1 && j == APP_USB_MAX-2) {
#if DBGPOLL
                            LOG_PRINT(error_e,"Device Unplugged %d mounted at %d free mpoint index %d\n",app_usb_scsiview[i].dev, app_usb_scsiview[i].mpoint, app_usb_scsiview[i].mpoint - 'a' );
#endif								
                            app_usb_scsiview[i].dev = -1;
                            strcpy(app_usb_scsiview[i].vendor, "\0");
                            strcpy(app_usb_scsiview[i].serial, "\0");
                            app_usb_available_mpoint[app_usb_scsiview[i].mpoint - 'a' ] = 1;
                            break;
                        }
                    }
                }
#if DBGPOLL
                for( i= 0; i < APP_USB_MAX-1; i++)
                    LOG_PRINT(error_e, "Previous  dev %d, mpoint %d --- Current  dev %d, mpoint %d\n", app_usb_scsiview[i].dev,app_usb_scsiview[i].mpoint,app_usb_scsiview_current[i].dev, app_usb_scsiview_current[i].mpoint);
#endif 				
                /*Scan the current array where we left only newly plugged device*/
                for (j = 0; j < APP_USB_MAX-1; j++){
                    if(app_usb_scsiview_current[j].dev != -1){
                        for(i = 0; app_usb_scsiview[i].dev != -1 && i < APP_USB_MAX-1; i++);
                        strcpy(app_usb_scsiview[i].vendor, app_usb_scsiview_current[j].vendor);
                        strcpy(app_usb_scsiview[i].serial, app_usb_scsiview_current[j].serial);
                        app_usb_scsiview[i].dev = app_usb_scsiview_current[j].dev;
                        for(k = 0; k < APP_USB_MAX-1; k++)
                            if (app_usb_available_mpoint[k]){
                                app_usb_available_mpoint[k] = 0; /*mpoint busy*/
                                app_usb_scsiview[i].mpoint = 'a' + k;
                                break;
                            }
                        /*Clean current data structure*/
                        strcpy(app_usb_scsiview_current[j].vendor, "\0");
                        strcpy(app_usb_scsiview_current[j].serial, "\0");
                        app_usb_scsiview_current[j].dev = -1;
                    }


                }
#if DBGPOLL
                for( i= 0; i < APP_USB_MAX-1; i++)
                    LOG_PRINT(error_e, "Final Previous  dev %d, mpoint %d --- Current  dev %d, mpoint %d\n", app_usb_scsiview[i].dev,app_usb_scsiview[i].mpoint, app_usb_scsiview_current[i].dev, app_usb_scsiview_current[i].mpoint);
#endif 
            }/*end else*/

            fp= fopen(USB_DEVICES_FILE, "r");
            if (fp == NULL)
            {
                LOG_PRINT(error_e, "cannot open '%s' [%s]\n", USB_DEVICES_FILE, strerror(errno));
                return -1;
            }

            FILE * fp2 = fopen("/tmp/dev.log", "w");
            while(fgets(line,MAX_LINE_SIZE,fp)!=NULL){
                if (strncmp(line, "T:", 2) == 0 || strncmp(line, "S:", 2) == 0)
                {
                    fprintf(fp2, "%s", line);
                }
            }
            fclose(fp);
            fclose(fp2);

            fp= fopen("/tmp/dev.log", "r");
            if (fp == NULL)
            {
                LOG_PRINT(error_e, "cannot open /tmp/dev.log [%s]\n", strerror(errno));
                return -1;
            }
            hubkey = 0;

#if DBGPOLL
            for( i= 0; i < APP_USB_MAX-1; i++)
                LOG_PRINT(error_e, "Final vendor %s, serial %s ---- i=%d \n", app_usb_scsiview[i].vendor,app_usb_scsiview[i].serial, i);
#endif		

            /* Init */
            for (i = 0; i < APP_USB_MAX+1; i++ )
                app_usb_status[i] = 0;

            while(fgets(line,MAX_LINE_SIZE,fp)!=NULL){
#if DBGPOLL
                LOG_PRINT(error_e,"%s\n", line);
#endif
                if(*line == 'T')
                    devinfo.kind = 1;
                else
                    devinfo.kind = 2;

                switch(devinfo.kind){
                case 1: sscanf(line,"T:  Bus=%*s Lev=%2hu Prnt=%2hu Port=%2hu Cnt=%2hu Dev#=%3hu Spd=%*s  MxCh=%2hu", &devinfo.lev, &devinfo.prnt, &devinfo.port, &devinfo.cnt, &devinfo.dev, &devinfo.mxch);
                    if (devinfo.lev == 0)
                        continue;
                    else
                        switch(devinfo.lev) {
                        case 1: if (devinfo.mxch > 1 )
                                app_usb_status[0] = devinfo.mxch;
                            else
                                app_usb_status[0] = 1;
#if DBGPOLL									
                            LOG_PRINT(error_e,"PARSED:Lev=%2hu PRNT=%2hu Port=%2hu Cnt=%2hu Dev#=%3hu MxCh=%2hu\n", devinfo.lev, devinfo.prnt, devinfo.port, devinfo.cnt, devinfo.dev, devinfo.mxch);
#endif								

                            break;
                        default:
                            if (!devinfo.mxch && hubkey && app_usb_status[hubkey] == devinfo.prnt ) {
                                app_usb_status[hubkey] = devinfo.dev;
                                hubkey = 0;
                            }
                            else if (!devinfo.mxch && devinfo.cnt )
                                app_usb_status[devinfo.port + 1] = devinfo.dev;
                            else
                                if(devinfo.mxch == 1){
                                    hubkey = devinfo.port + 1;
                                    hubkey_bkp = hubkey;
                                    app_usb_status[devinfo.port + 1] = devinfo.dev;
                                }
                                else
                                    app_usb_status[APP_USB_MAX+1] = 10; /*Detected another multiport-hub, only one supported!!!!!!!!!!!!*/
#if DBGPOLL										
                            LOG_PRINT(error_e,"PARSED:Lev=%2hu PRNT=%2hu Port=%2hu Cnt=%2hu Dev#=%3hu MxCh=%2hu\n", devinfo.lev, devinfo.prnt, devinfo.port, devinfo.cnt, devinfo.dev, devinfo.mxch);
#endif								
                            break;
                        } /*end switch on devinfo.lev*/
                    devinfo.serial[0]='\0';
                    devinfo.vendor[0]='\0';
                    break;

                default:

                    if (devinfo.lev == 0 || devinfo.mxch != 0)
                        continue;
                    else {
                        ll = strlen(line);
                        if(line[ll - 1] == '\n')
                            line[ll - 1] = '\0';
                        if ((l = strstr(line, "Manufacturer=" )) != NULL ){
                            l = l + strlen("Manufacturer=");
                            strcpy(devinfo.vendor, l);
#if DBGPOLL									
                            LOG_PRINT(error_e,"devinfo.vendor = %s\n", devinfo.vendor);
#endif									
                        }
                        else if (strstr(line, "Product=" ) != NULL );

                        else if ((l = strstr(line, "SerialNumber=" )) != NULL ){
                            l = l + strlen("SerialNumber=");
                            strcpy(devinfo.serial, l);
#if DBGPOLL									
                            LOG_PRINT(error_e,"devinfo.serial = %s\n", devinfo.serial);
#endif									
                        }
                        else
                            app_usb_status[APP_USB_MAX+1] = 20; /* error in parsing info for usb key*/
                    }

                    current = ftell(fp);

                    /*If we find a line that starts with T we know that we
                              finished the processing for current device so we save file
                              pointer position to recover to reprocess the T line as the
                              first line of a new device*/

                    if(fgets(line, MAX_LINE_SIZE, fp) == NULL || *line =='T'){
#if DBGPOLL						
                        LOG_PRINT(error_e,"tryng to find a match for: vendor = %s, serial = %s\n", devinfo.vendor, devinfo.serial );
#endif						  
                        for ( i = 0; i < APP_USB_MAX-1; i++ )
                            if (strcmp(devinfo.serial, app_usb_scsiview[i].serial) == 0){
#if DBGPOLL
                                LOG_PRINT(error_e,"Current checked scsi device: i = %d app_usb_scsiview[i].serial = %s, app_usb_scsiview[i].vendor = %s\n", i, app_usb_scsiview[i].serial, app_usb_scsiview[i].vendor );
#endif
                                if(hubkey_bkp && !hubkey){
                                    app_usb_status[hubkey_bkp] = (int)(app_usb_scsiview[i].mpoint);
                                    hubkey_bkp = 0;
                                }
                                else
                                    app_usb_status[devinfo.port + 1] = (int)(app_usb_scsiview[i].mpoint);
                                break;
                            }

                    }
                    fseek(fp, current, SEEK_SET);


                    break;

                } /* end switch on devinfo.kind */



            } /*end while*/

            fclose(fp);
            remove("/tmp/dev.log");

#if DBGPOLL
            LOG_PRINT(error_e, "\n \n \n Configurazione attuale:\n");
            for (i = 0; i < APP_USB_MAX+1; i++ )
                LOG_PRINT(error_e, "app_usb_status[%d] = %hu\n", i, app_usb_status[i]);
            LOG_PRINT(error_e,"scsi = %d\n",scsi);
            for (i=0; i< APP_USB_MAX; i++)
                LOG_PRINT(error_e, "scsi%d %s, %s\n",i, app_usb_scsiview[i].vendor, app_usb_scsiview[i].serial );
#endif		
            return 0;
        } /*end if -- found devices file modified hence, elaborate it!*/
    } /*end else*/
}



/*PLC Views*/

unsigned short
Usb_on( unsigned short index )
{
    return app_usb_mount ( index );
}

unsigned short
Usb_off( unsigned short index )
{
    return app_usb_umount ( index );
}
char *
Usb_mpoint( unsigned short index )
{
    return app_usb_mpoint_return ( index );
}

unsigned short
Usb_copy(char *srcfile, char *dstfile, unsigned short src_index, unsigned short dst_index )
{
    return app_usb_file_copy(srcfile, dstfile, src_index, dst_index );
}

unsigned short
Usb_delete(char *filename, unsigned short index )
{
    return app_usb_file_delete (filename, index );
}


unsigned short
Usb_mkdir(char *dirname, unsigned short index )
{
    return app_usb_dir_create (dirname, index );
}

unsigned short
Usb_diskcopy(char *srcfile, char *dstfile, unsigned short src_index )
{
    return app_usb_file_diskcopy(srcfile, dstfile, src_index );
}

unsigned short
HMItextReport(char *filename, unsigned short index )
{
    return app_usb_file_report(filename, index );
}

/*	TEST
    int main(void)
    {
    app_usb_init();
    int usb = 1;
    int trial = 0;
    LOG_PRINT(error_e," Init usb system ok\n");
    while(1)
    {
    app_usb_status_read();
    if ( usb == 0 )
    LOG_PRINT(error_e," USB system ok\n");
    else if	( usb == -1 )
    LOG_PRINT(error_e," USB system ERROR\n");

    sleep(10);
#if 1
USBstatus[0] = 1;
USBstatus[1] = 97;		
if(app_usb_status[0] == 1 && trial == 0)
{
Usb_copy("/local/root/pippo", "gerry", 0, 1);
trial = 1;
}
#endif
}
return 0;
}
 */
