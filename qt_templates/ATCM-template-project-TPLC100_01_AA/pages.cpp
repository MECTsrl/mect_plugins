/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Selector of the new pages
 */
#include <math.h>

#include "app_logprint.h"
#include "crosstable.h"
#include "global_var.h"
#include "utility.h"
#include "pages.h"

void printVncDisplayString(char * vncString)
{
    // For TPLC050 and TPLC100 ignore system file [/sys/class/graphics/fb0/virtual_size] but use user Project Variables  WIDTH, HEIGHT, ROTATION as before MS 3.3.7
    sprintf(vncString, "Multi: VNC:0:size=%dx%d Transformed:rot%d", WIDTH, HEIGHT, ROTATION);
    userPageList 
            << "system_ini"
            << "page100"
               /* add here the label of the new page */
               ;
    userPageList.removeDuplicates();
    
}

int create_page_nb(page ** p, int pageNb)
{
    switch (pageNb)
    {
    case 0x0:
        *p = (page *)(new page0);
        break;
    case 0x100:
        *p = (page *)(new page100);
        break;
        /* add here the case labeled with the HEX id of the new pages */
    default:
        //printf ("Fatal error: cannot find page %d\n", pageNb);
        *p = NULL;
        return 1;
        break;
    }
    return 0;
}


