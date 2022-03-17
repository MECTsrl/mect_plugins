#include "screensaver.h"
#include "app_logprint.h"

#include <string.h>
#include "common.h"
#include "global_var.h"
#include "cross_table_utility.h"

#define BLANK_SCREEN_FILE "/sys/class/graphics/fb0/blank"

void ScreenSaver::restore()
{
    //turn backlight on
    
    int         brightness_level;
    char        command[256] = "";
    u_int32_t   uModel = 0;

    //get current settings for backlight
    FILE * fp = fopen(BACKLIGHT_FILE_LOCAL, "r");
    
    if (fp == NULL)
    {
        LOG_PRINT(verbose_e, "no local configuration file for screensaver found using defaults \n");
        brightness_level = DEF_BACKLIGHT_LEVEL;
    }
    else
    {
        while(fgets(command,256,fp)!=NULL)
        {
            LOG_PRINT(verbose_e, "BACKLIGHT: %s\n", command);
            sscanf(command,"echo %d %*s", &brightness_level);
        }
        fclose(fp);
    }
    
    if (brightness_level < MIN_BACKLIGHT_LEVEL)
    {
        brightness_level = MIN_BACKLIGHT_LEVEL;
    }
    else if ( brightness_level > MAX_BACKLIGHT_LEVEL)
    {
        brightness_level = 100;
    }
    // Get Model Number, at startup is 0
    readFromDbQuick(ID_PLC_PRODUCT_ID, (int *)&uModel);
    uModel = uModel >> 8;
    // fprintf(stderr, "Model Family: 0x%08x - ON Screen\n", uModel);
    // Patch for 100802 Family
    if (uModel == 0x100802)  {
        sprintf (command, "echo %d > %s", 0, BLANK_SCREEN_FILE);
    }
    else  {
        sprintf (command, "echo %d > %s", brightness_level, BACKLIGHT_FILE_SYSTEM);
    }
    system(command);
    
    LOG_PRINT(warning_e, "EXITING SCREENSAVER\n");
}

bool ScreenSaver::save(__attribute__((unused)) int level)
{
    //turn backlight off
    int         brightness_level = 10;
    char        command[256] = "";
    unsigned    uModel = 0;

    // Get Model Number, at startup is 0
    readFromDbQuick(ID_PLC_PRODUCT_ID, (int *)&uModel);
    uModel = uModel >> 8;
    // fprintf(stderr, "Model Family: 0x%08x - OFF Screen\n", uModel);
    // Patch for 100802 Family
    if (uModel == 0x100802)  {
        sprintf (command, "echo %d > %s", 1, BLANK_SCREEN_FILE);
    }
    else  {
        sprintf (command, "echo %d > %s", brightness_level, BACKLIGHT_FILE_SYSTEM);
    }
    system(command);
    
    LOG_PRINT(warning_e, "ENTERING SCREENSAVER\n");
    return true;
}
