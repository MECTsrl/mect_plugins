/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief HMI Main function
 */
#include <QApplication>
#include <QDate>
#include <getopt.h>

#include "page0.h"
#include "common.h"
#include "main.h"
#include "app_logprint.h"
#include "utility.h"
#include "global_var.h"

#include "system_ini.h"
#include "comm_status.h"
#include "cross_table_utility.h"
#include "data_manager.h"
#include "display_settings.h"
#include "info.h"
#include "main.h"
#include "menu.h"
#include "options.h"
#include "page0.h"
#include "time_set.h"

#ifdef ENABLE_ALARMS
#include "alarms.h"
#include "alarms_history.h"
#endif

#ifdef ENABLE_STORE
#include "store.h"
#include "store_filter.h"
#endif

#ifdef ENABLE_TREND
#include "trend.h"
#include "trend_other.h"
#include "trend_option.h"
#include "trend_range.h"
#endif

#ifdef ENABLE_RECIPE
#include "recipe.h"
#include "recipe_select.h"
#endif

/* Long options */
static struct option long_options[] = {
    {"version", no_argument,        NULL, 'v'},
    {"qt", no_argument,        NULL, 'q'},
    {"qt", no_argument,        NULL, 'w'},
    {"qt", no_argument,        NULL, 's'},
    {"qt", no_argument,        NULL, 'd'},
    {"qt", no_argument,        NULL, 'i'},
    {"qt", no_argument,        NULL, 'p'},
    {"qt", no_argument,        NULL, 'l'},
    {"qt", no_argument,        NULL, 'a'},
    {"qt", no_argument,        NULL, 'y'},
    {NULL,      no_argument,        NULL,  0}
};

/*
 * Short options.
 * FIXME: KEEP THEIR LETTERS IN SYNC WITH THE RETURN VALUE
 * FROM THE LONG OPTIONS!
 */
static char short_options[] = "vqwsdiplay";

static int application_options(int argc, char *argv[])
{
    int option_index = 0;
    int c = 0;
    
    if (argc <= 0)
        return 0;
    
    if (argv == NULL)
        return 1;
    
    while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
        switch (c) {
        case 'v':
            printf("%s version: %s\n", argv[0], VERSION);
            exit(0);
            break;
        default:
            break;
        }
    }
    
    return 0;
}

/**
 * @brief main
 */
int main(int argc, char *argv[])
{
    /* parse the command line option */
    if (application_options(argc, argv) != 0) {
        LOG_PRINT(error_e, "%s: command line option error.\n", __func__);
        return 1;
    }
    
    /* print the version */
    LOG_PRINT_NO_INFO(info_e, "#######################\n");
    LOG_PRINT_NO_INFO(info_e, "# Version: %10s #\n", VERSION);
    LOG_PRINT_NO_INFO(info_e, "#######################\n");
    
    /* instantiate the GUI application object */

    char vncDisplay[64];
    printVncDisplayString(vncDisplay);

    int myargc = 4;
    char *myargv[] =
    {
        argv[0],
        strdup("-qws"),
        strdup("-display"),
        vncDisplay
    };

    QApplication app(myargc, myargv);
    
    /* load the library icons */
    Q_INIT_RESOURCE(atcmicons);

    /* initialize the application (load configurations and start threads) */
    initialize();

    /* start page 0 (the splash screen) */
    page0 w;
    w.SHOW();
    w.reload();
    
    /* start the buzzer event filter */
    app.installEventFilter(new MyEventFilter());
    
    /* start the GUI application */
    app.exec();
    
    /* finalize the application (stop threads) */
    finalize();

    return 0;
}
