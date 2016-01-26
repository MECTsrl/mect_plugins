#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#include "app_config.h"

/*
 * Constant definitions
 */



enum app_conf_section_e {
    APP_CONF_NONE = 0,
    APP_CONF_IRQ0,              /* IRQ0 section */
    APP_CONF_IRQ1,              /* IRQ1 section */
    APP_CONF_PLC0,              /* PLC0 section */
    APP_CONF_PLC1,              /* PLC1 section */
    APP_CONF_PLC2,              /* PLC2 section */
    APP_CONF_PLC3,              /* PLC3 section */
    APP_CONF_HMI,               /* HMI section */
    APP_CONF_ST,                /* MECT serial test protocol (slave) section */
    APP_CONF_MECT,              /* MECT serial protocol (master) section */
    APP_CONF_MB,                /* MODBUS protocol section */
    APP_CONF_WD,                /* Watch dog section */
    APP_CONF_LAST_ELEM          /* last section */
};

/*
 * Global variabiles
 */

app_conf_t app_conf = {
    .irq0 = {
        .enabled = 0,
        .outmask = 0xffffffff,
        .encmask = 0,
        .anomask = 0
    },

    .irq1 = {
        .enabled = 0,
        .outmask = 0xffffffff,
        .encmask = 0,
        .anomask = 0
    },

    .plc0 = {
        .cycle = 10,
        .can = APP_CONF_CANPLC_NONE,
        .outmask = 0xffffffff,
        .encmask = 0xffffffff,
        .anomask = 0xffffffff,
        .label =""
    },

    .plc1 = {
        .cycle = -1,
        .can = APP_CONF_CANPLC_NONE,
        .outmask = 0x00000000,
        .encmask = 0x00000000,
        .anomask = 0x00000000,
        .label =""
    },

    .plc2 = {
        .cycle = -1,
        .can = APP_CONF_CANPLC_NONE,
        .outmask = 0x00000000,
        .encmask = 0x00000000,
        .anomask = 0x00000000,
        .label =""
    },

    .plc3 = {
        .cycle = -1,
        .can = APP_CONF_CANPLC_NONE,
        .outmask = 0x00000000,
        .encmask = 0x00000000,
        .anomask = 0x00000000,
        .label =""
    },

    .hmi = {
        .cycle = -1,
        .can = APP_CONF_CANPLC_NONE,
        .outmask = 0x00000000,
        .encmask = 0x00000000,
        .anomask = 0x00000000
    },

    .st = {
        .enabled = 0
    },

    .mect = {
        .enabled = 0,
        .debug = 0
    },

    .wd = {
        .enabled = 0
    },

    .mb = {
        .enabled = 0,
        .ascii = 0,
        .rtu = 0,
        .baud = -1,
        .databits = -1,
        .parity = -1,
        .stopbits = -1,
        .ignore_echo = -1
    },

    .free_run  = {
        .enabled = 0
    }
};

app_conf_build_ids_t app_conf_build_ids = {
    .main = {
        .tag = "main",
        .id = ""
    },
    .cpld = {
        .tag = "CPLD",
        .id = ""
    },
    .kernel = {
        .tag = "kernel",
        .id = ""
    },
    .timer = {
        .tag = "timer",
        .id = ""
    },
    .can = {
        .tag = "CAN",
        .id = ""
    },
    .retentive = {
        .tag = "retentive",
        .id = ""
    },
    .root = {
        .tag = "root",
        .id = ""
    },
    .local = {
        .tag = "local",
        .id = ""
    },
    .boot = {
        .tag = "boot",
        .id = ""
    }
};

static char *app_conf_mac = NULL;
static char *app_conf_serial = NULL;
static char *app_conf_ipaddr = NULL;
static char *app_conf_netmask = NULL;
static char *app_conf_gateway = NULL;
static char *app_conf_dns1 = NULL;
static char *app_conf_dns2 = NULL;
static char *app_conf_lastupd = NULL;

/*
 * Local function prototypes
 */

static char *app_property_name_check(char *line, char *prop);
static char *app_expect_equal(const char *fn, char *line, unsigned ln, char *cf);

/**
 * @brief Grab a net conf item
 *
 * @param char * item_name : item name;
 * @param char * item_pointer : poiter to the item;
 *
 * @return          NULL if error or pointer to item string
 *
 * @ingroup config
 */
char *app_netconf_item_get(char ** item_pointer, const char * item_name)
{
#define MAX_LINE_SIZE       81

    char line[MAX_LINE_SIZE];
    FILE *cf = NULL;
    int len = 0;

    cf = fopen(APP_CONFIG_IPADDR_FILE, "r");
    if (cf == NULL) {
        perror(APP_CONFIG_IPADDR_FILE);

        return NULL;
    }

    while ((fgets(line, MAX_LINE_SIZE, cf))!= NULL ) {

        if (strstr(line, item_name) != NULL)
            break;
    }
    fclose(cf);

    len = strlen(line);

    if (line[len - 1] == '\n')
        line[len - 1] = '\0';

    if (*item_pointer != NULL)
        free(*item_pointer);

    if (strchr(line, '=') != NULL)
    {
        *item_pointer = strdup(strchr(line, '=') + 1);
    }
    else
    {
        *item_pointer = strdup(line);
    }
    assert(*item_pointer != NULL);


    return *item_pointer;

#undef MAX_LINE_SIZE
}

/**
 * @brief Grab the MAC address
 *
 * @return          NULL if error or pointer to MAC string
 *
 * @ingroup config
 */
char *
app_build_mac_get(void)
{
    return app_netconf_item_get(&app_conf_mac, "MAC0");
}

/**
 * @brief Grab the ip address
 *
 * @return          NULL if error or pointer to serial number string
 *
 * @ingroup config
 */
char *
app_build_ipaddr_get(void)
{
    return app_netconf_item_get(&app_conf_ipaddr, "IPADDR0");
}

/**
 * @brief Grab the NetMask address
 *
 * @return          NULL if error or pointer to netmask string
 *
 * @ingroup config
 */
char *app_build_netmask_get(void)
{
    return app_netconf_item_get(&app_conf_netmask, "NETMASK0");
}

/**
 * @brief Grab the Gateway address
 *
 * @return          NULL if error or pointer to gateway string
 *
 * @ingroup config
 */
char *app_build_gateway_get(void)
{
    return app_netconf_item_get(&app_conf_gateway, "GATEWAY0");
}

/**
 * @brief Grab the DNS 1 address
 *
 * @return          NULL if error or pointer to DNS 1 string
 *
 * @ingroup config
 */
char *app_build_dns1_get(void)
{
    return app_netconf_item_get(&app_conf_dns1, "NAMESERVER01");
}

/**
 * @brief Grab the DNS 2 address
 *
 * @return          NULL if error or pointer to DNS 2 string
 *
 * @ingroup config
 */
char *app_build_dns2_get(void)
{
    return app_netconf_item_get(&app_conf_dns2, "NAMESERVER02");
}

/**
 * @brief Grab the serial number
 *
 * @return          NULL if error or pointer to serial number string
 *
 * @ingroup config
 */
char *
app_build_serial_get(void)
{
#define MAX_LINE_SIZE       81

    char line[MAX_LINE_SIZE];
    FILE *cf = NULL;

    cf = fopen(APP_CONFIG_SERIAL_FILE, "r");
    if (cf == NULL) {
        perror(APP_CONFIG_SERIAL_FILE);

        return NULL;
    }

    if (fgets(line, MAX_LINE_SIZE, cf) == line) {
        int len = 0;

        len = strlen(line);

        if (line[len - 1] == '\n')
            line[len - 1] = '\0';

        if (app_conf_serial != NULL)
            free(app_conf_serial);

        app_conf_serial = strdup(line);
        assert(app_conf_serial != NULL);
    }
    fclose(cf);
    return app_conf_serial;

#undef MAX_LINE_SIZE
}

/**
 * Grab the last update
 *
 * @return          NULL if error or pointer to serial number string
 *
 * @ingroup config
 */
char *
app_last_update_get(void)
{
#define MAX_LINE_SIZE       81

    char line[MAX_LINE_SIZE];
    FILE *cf = NULL;

    cf = fopen(APP_CONFIG_LASTUPD_FILE, "r");
    if (cf == NULL) {
        perror(APP_CONFIG_LASTUPD_FILE);

        return NULL;
    }

    if (fgets(line, MAX_LINE_SIZE, cf) == line) {
        int len = 0;

        len = strlen(line);

        if (line[len - 1] == '\n')
            line[len - 1] = '\0';

        if (app_conf_lastupd != NULL)
            free(app_conf_lastupd);

        if (strchr(line, '/') == NULL)
        {
            char tmp[MAX_LINE_SIZE];
            sprintf(tmp, "%c%c%c%c/%c%c/%c%c",
                    line[0],
                    line[1],
                    line[2],
                    line[3],
                    line[4],
                    line[5],
                    line[6],
                    line[7]
                    );
            strcpy(line, tmp);
        }

        app_conf_lastupd = strdup(line);
        assert(app_conf_lastupd != NULL);
    }
    fclose(cf);
    return app_conf_lastupd;

#undef MAX_LINE_SIZE
}

/**
 * @brief Grab the build IDs for the main components of the system
 * from the file filled at boot time.  The file format is:
 * tag build_ID.
 *
 * @return          NULL if error or pointer to struct with build IDs
 *
 * @ingroup config
 */
app_conf_build_ids_t *
app_build_ids_get(void)
{
#define MAX_LINE_SIZE       81

    char line[MAX_LINE_SIZE];
    FILE *cf = NULL;

    cf = fopen(APP_CONFIG_BUILD_ID_FILE, "r");
    if (cf == NULL) {
        perror(APP_CONFIG_BUILD_ID_FILE);

        return NULL;
    }

    while (fgets(line, MAX_LINE_SIZE, cf) == line) {
        int len = 0;

        len = strlen(line);

        if (line[len - 1] == '\n')
            line[len - 1] = '\0';

        if (strstr(line, app_conf_build_ids.main.tag) == line)
            strncpy(app_conf_build_ids.main.id, &(line[5]), 16);
        else if (strstr(line, app_conf_build_ids.cpld.tag) == line)
            strncpy(app_conf_build_ids.cpld.id, &(line[5]), 16);
        else if (strstr(line, app_conf_build_ids.kernel.tag) == line)
            strncpy(app_conf_build_ids.kernel.id, &(line[7]), 16);
        else if (strstr(line, app_conf_build_ids.timer.tag) == line)
            strncpy(app_conf_build_ids.timer.id, &(line[6]), 16);
        else if (strstr(line, app_conf_build_ids.can.tag) == line)
            strncpy(app_conf_build_ids.can.id, &(line[4]), 16);
        else if (strstr(line, app_conf_build_ids.retentive.tag) == line)
            strncpy(app_conf_build_ids.retentive.id, &(line[10]), 16);
        else if (strstr(line, app_conf_build_ids.root.tag) == line)
            strncpy(app_conf_build_ids.root.id, &(line[5]), 16);
        else if (strstr(line, app_conf_build_ids.local.tag) == line)
            strncpy(app_conf_build_ids.local.id, &(line[6]), 16);
        else if (strstr(line, app_conf_build_ids.boot.tag) == line)
            strncpy(app_conf_build_ids.boot.id, &(line[5]), 16);
        else {
            fprintf(stderr, "%s: bad build ID line: %s\n", __func__, line);
            fflush(stderr);

            return NULL;
        }
    }
    fclose(cf);
    return &app_conf_build_ids;

#undef MAX_LINE_SIZE
}

/**
 * @brief Check whether the given line starts with the given property
 * name.  The property name is terminated by ' ', '\t', or '='.
 *
 * @param line      input line
 * @param prop      property name
 *
 * @return          NULL if does not match
 *                  pointer to end of property name in line if matches
 *
 * @ingroup config
 */
static char *
app_property_name_check(char *line, char *prop)
{
    unsigned pl = 0;

    assert(line != NULL);
    assert(prop != NULL);

    pl = strlen(prop);
    if ((strstr(line, prop) == line) && (strspn(&line[pl], " =\t") > 0))
        return &line[pl];
    else
        return NULL;
}

/**
 * Check whether the next non whitespace char is '=' in the given line
 *
 * @param fn        calling function name
 * @param line      the line to scan
 * @param ln        line number
 * @param cf        configuration file name
 *
 * @return          pointer to the start of the first identifier after '='
 *                  NULL on failure
 *
 * @ingroup config
 */
static char *
app_expect_equal(const char *fn, char *line, unsigned ln, char *cf)
{
    assert(fn != NULL);
    assert(line != NULL);
    assert(ln > 0);
    assert(cf != NULL);

    line += strspn(line, " \t");
    if (*line != '=') {
        fprintf(stderr, "%s: expecting `=' after identifier on line %d in file %s\n", fn, ln, cf);

        return NULL;
    }
    else
        line++;
    line += strspn(line, " \t");

    return line;
}

/**
 * @brief Load the application configuration settings from the
 * configuration file
 *
 * @return      0 for success; non-0 code for error
 *
 * @ingroup config
 */
int
app_config_load(void)
{
#define MAX_LINE_SIZE       81

    char line[MAX_LINE_SIZE];
    FILE *cf = NULL;
    unsigned section = APP_CONF_NONE;
    unsigned ln = 0;

    cf = fopen(APP_CONFIG_FILE, "r");
    if (cf == NULL) {
        perror(APP_CONFIG_FILE);

        return 1;
    }

    for (ln = 1; fgets(line, MAX_LINE_SIZE, cf) == line; ln++) {
        char *l = NULL;
        unsigned ll = 0;

        ll = strlen(line);

        if (line[ll - 1] == '\n')
            line[ll - 1] = '\0';

        l = line;

        l += strspn(l, " \t");      /* Skip white space */

        if (*l == '[') {            /* Category start */
            l++;

            if (strstr(l, "IRQ0]") == l)
                section = APP_CONF_IRQ0;
            else if (strstr(l, "IRQ1]") == l)
                section = APP_CONF_IRQ1;
            else if (strstr(l, "PLC0]") == l)
                section = APP_CONF_PLC0;
            else if (strstr(l, "PLC1]") == l)
                section = APP_CONF_PLC1;
            else if (strstr(l, "PLC2]") == l)
                section = APP_CONF_PLC2;
            else if (strstr(l, "PLC3]") == l)
                section = APP_CONF_PLC3;
            else if (strstr(l, "HMI]") == l)
                section = APP_CONF_HMI;
            else if (strstr(l, "ST]") == l)
                section = APP_CONF_ST;
            else if (strstr(l, "MECT]") == l)
                section = APP_CONF_MECT;
            else if (strstr(l, "MODBUS]") == l)
                section = APP_CONF_MB;
            else if (strstr(l, "WD]") == l)
                section = APP_CONF_WD;
            else
                fprintf(stderr, "%s: Unknown section on line %d in file %s\n", __func__, ln, APP_CONFIG_FILE);
        }
        else {
            char *tl = NULL;

            switch (section) {
            case APP_CONF_IRQ0:
                if ((tl = app_property_name_check(l, "enabled"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.irq0.enabled = strtoul(l, NULL, 0);
                    else
                    {
                        fclose(cf);
                        return 2;
                    }
                }
                else if ((tl = app_property_name_check(l, "outmask"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.irq0.outmask = strtoul(l, NULL, 0);
                    else
                    {
                        fclose(cf);
                        return 3;
                    }
                }
                else if ((tl = app_property_name_check(l, "encmask"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.irq0.encmask = strtoul(l, NULL, 0);
                    else
                    {
                        fclose(cf);
                        return 4;
                    }
                }
                else if ((tl = app_property_name_check(l, "anomask"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.irq0.anomask = strtoul(l, NULL, 0);
                    else
                    {
                        fclose(cf);
                        return 5;
                    }

                }
                else if (tl != NULL) {
                    fprintf(stderr, "%s: unknown property %s for IRQ0\n", __func__, tl);
                    fclose(cf);
                    return 6;
                }

                break;

            case APP_CONF_IRQ1:
                if ((tl = app_property_name_check(l, "enabled"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.irq1.enabled = strtoul(l, NULL, 0);
                    else
                    {
                        fclose(cf);
                        return 7;
                    }
                }
                else if ((tl = app_property_name_check(l, "outmask"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.irq1.outmask = strtoul(l, NULL, 0);
                    else
                    {
                        fclose(cf);
                        return 8;
                    }
                }
                else if ((tl = app_property_name_check(l, "encmask"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.irq1.encmask = strtoul(l, NULL, 0);
                    else
                    {
                        fclose(cf);
                        return 9;
                    }
                }
                else if ((tl = app_property_name_check(l, "anomask"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.irq1.anomask = strtoul(l, NULL, 0);
                    else
                    {
                        fclose(cf);
                        return 10;
                    }
                }
                else if (tl != NULL) {
                    fprintf(stderr, "%s: unknown property %s for IRQ1\n", __func__, tl);
                    fclose(cf);
                    return 11;
                }

                break;

            case APP_CONF_PLC0:
                if ((tl = app_property_name_check(l, "cycle"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.plc0.cycle = atoi(l);
                    else
                    {
                        fclose(cf);
                        return 12;
                    }
                }
                else if ((tl = app_property_name_check(l, "outmask"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.plc0.outmask = strtoul(l, NULL, 0);
                    else
                    {
                        fclose(cf);
                        return 13;
                    }
                }
                else if ((tl = app_property_name_check(l, "encmask"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.plc0.encmask = strtoul(l, NULL, 0);
                    else
                    {
                        fclose(cf);
                        return 14;
                    }
                }
                else if ((tl = app_property_name_check(l, "anomask"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.plc0.anomask = strtoul(l, NULL, 0);
                    else
                    {
                        fclose(cf);
                        return 15;
                    }
                }
                else if ((tl = app_property_name_check(l, "can"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL) {
                        app_conf.plc0.can = atoi(l);
                        if (app_conf.plc0.can & APP_CONF_CANALLPLC) {
                            fprintf(stderr, "%s: illegal CANopen net associated to PLC0: 0x%02x. Discarded.\n", __func__, (unsigned int)app_conf.plc0.can);
                            app_conf.plc0.can = APP_CONF_CANPLC_NONE;
                        }
                    }
                    else
                    {
                        fclose(cf);
                        return 16;
                    }

                }
                if ((tl = app_property_name_check(l, "label"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        strcpy(app_conf.plc0.label,l);
                    else
                    {
                        fclose(cf);
                        return 17;
                    }

                }
                else if (tl != NULL) {
                    fprintf(stderr, "%s: unknown property %s for PLC0\n", __func__, tl);
                    fclose(cf);
                    return 18;
                }

                break;

            case APP_CONF_PLC1:
                if ((tl = app_property_name_check(l, "cycle"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.plc1.cycle = atoi(l);
                    else
                    {
                        fclose(cf);
                        return 19;
                    }

                }
                else if ((tl = app_property_name_check(l, "outmask"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.plc1.outmask = strtoul(l, NULL, 0);
                    else
                    {
                        fclose(cf);
                        return 20;
                    }

                }
                else if ((tl = app_property_name_check(l, "encmask"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.plc1.encmask = strtoul(l, NULL, 0);
                    else
                    {
                        fclose(cf);
                        return 21;
                    }

                }
                else if ((tl = app_property_name_check(l, "anomask"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.plc1.anomask = strtoul(l, NULL, 0);
                    else
                    {
                        fclose(cf);
                        return 22;
                    }

                }
                else if ((tl = app_property_name_check(l, "can"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL) {
                        app_conf.plc1.can = atoi(l);
                        if (app_conf.plc1.can & APP_CONF_CANALLPLC) {
                            fprintf(stderr, "%s: illegal CANopen net associated to PLC2: 0x%02x. Discarded.\n", __func__, (unsigned int)app_conf.plc1.can);
                            app_conf.plc1.can = APP_CONF_CANPLC_NONE;
                        }
                    }
                    else
                    {
                        fclose(cf);
                        return 23;
                    }

                }
                if ((tl = app_property_name_check(l, "label"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        strcpy(app_conf.plc1.label, l);
                    else
                    {
                        fclose(cf);
                        return 24;
                    }

                }
                else if (tl != NULL) {
                    fprintf(stderr, "%s: unknown property %s for PLC2\n", __func__, tl);
                    fclose(cf);
                    return 25;
                }

                break;

            case APP_CONF_PLC2:
                if ((tl = app_property_name_check(l, "cycle"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.plc2.cycle = atoi(l);
                    else
                    {
                        fclose(cf);
                        return 26;
                    }
                }
                else if ((tl = app_property_name_check(l, "outmask"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.plc2.outmask = strtoul(l, NULL, 0);
                    else
                    {
                        fclose(cf);
                        return 27;
                    }
                }
                else if ((tl = app_property_name_check(l, "encmask"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.plc2.encmask = strtoul(l, NULL, 0);
                    else
                    {
                        fclose(cf);
                        return 28;
                    }
                }
                else if ((tl = app_property_name_check(l, "anomask"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.plc2.anomask = strtoul(l, NULL, 0);
                    else
                    {
                        fclose(cf);
                        return 29;
                    }
                }
                else if ((tl = app_property_name_check(l, "can"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL) {
                        app_conf.plc2.can = atoi(l);
                        if (app_conf.plc2.can & APP_CONF_CANALLPLC) {
                            fprintf(stderr, "%s: illegal CANopen net associated to PLC2: 0x%02x. Discarded.\n", __func__, (unsigned int)app_conf.plc2.can);
                            app_conf.plc2.can = APP_CONF_CANPLC_NONE;
                        }
                    }
                    else
                    {
                        fclose(cf);
                        return 29;
                    }

                }
                if ((tl = app_property_name_check(l, "label"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        strcpy(app_conf.plc2.label, l);
                    else
                    {
                        fclose(cf);
                        return 30;
                    }

                }
                else if (tl != NULL) {
                    fprintf(stderr, "%s: unknown property %s for PLC2\n", __func__, tl);
                    fclose(cf);
                    return 31;
                }

                break;

            case APP_CONF_PLC3:
                if ((tl = app_property_name_check(l, "cycle"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.plc3.cycle = atoi(l);
                    else
                    {
                        fclose(cf);
                        return 32;
                    }
                }
                else if ((tl = app_property_name_check(l, "outmask"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.plc3.outmask = strtoul(l, NULL, 0);
                    else
                    {
                        fclose(cf);
                        return 33;
                    }
                }
                else if ((tl = app_property_name_check(l, "encmask"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.plc3.encmask = strtoul(l, NULL, 0);
                    else
                    {
                        fclose(cf);
                        return 34;
                    }
                }
                else if ((tl = app_property_name_check(l, "anomask"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.plc3.anomask = strtoul(l, NULL, 0);
                    else
                    {
                        fclose(cf);
                        return 35;
                    }
                }
                else if ((tl = app_property_name_check(l, "can"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL) {
                        app_conf.plc3.can = atoi(l);
                        if (app_conf.plc3.can & APP_CONF_CANALLPLC) {
                            fprintf(stderr, "%s: illegal CANopen net associated to PLC3: 0x%02x. Discarded.\n", __func__, (unsigned int)app_conf.plc3.can);
                            app_conf.plc3.can = APP_CONF_CANPLC_NONE;
                        }
                    }
                    else
                    {
                        fclose(cf);
                        return 36;
                    }
                }
                if ((tl = app_property_name_check(l, "label"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        strcpy(app_conf.plc3.label, l);
                    else
                    {
                        fclose(cf);
                        return 37;
                    }
                }
                else if (tl != NULL) {
                    fprintf(stderr, "%s: unknown property %s for PLC3\n", __func__, tl);
                    fclose(cf);
                    return 38;
                }

                break;

            case APP_CONF_HMI:
                if ((tl = app_property_name_check(l, "cycle"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.hmi.cycle = atoi(l);
                    else
                    {
                        fclose(cf);
                        return 39;
                    }
                }
                else if ((tl = app_property_name_check(l, "outmask"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.hmi.outmask = strtoul(l, NULL, 0);
                    else
                    {
                        fclose(cf);
                        return 40;
                    }
                }
                else if ((tl = app_property_name_check(l, "encmask"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.hmi.encmask = strtoul(l, NULL, 0);
                    else
                    {
                        fclose(cf);
                        return 41;
                    }
                }
                else if ((tl = app_property_name_check(l, "anomask"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.hmi.anomask = strtoul(l, NULL, 0);
                    else
                    {
                        fclose(cf);
                        return 42;
                    }
                }
                else if ((tl = app_property_name_check(l, "can"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL) {
                        app_conf.hmi.can = atoi(l);
                        if (app_conf.hmi.can & APP_CONF_CANALLPLC) {
                            fprintf(stderr, "%s: illegal CANopen net associated to HMI: 0x%02x. Discarded.\n", __func__, (unsigned int)app_conf.hmi.can);
                            app_conf.hmi.can = APP_CONF_CANPLC_NONE;
                        }
                    }
                    else
                    {
                        fclose(cf);
                        return 43;
                    }
                }
                else if (tl != NULL) {
                    fprintf(stderr, "%s: unknown property %s for HMI\n", __func__, tl);
                    fclose(cf);
                    return 44;
                }

                break;

            case APP_CONF_ST:
                if ((tl = app_property_name_check(l, "enabled"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.st.enabled = atoi(l);
                    else
                    {
                        fclose(cf);
                        return 45;
                    }
                }
                else if (tl != NULL) {
                    fprintf(stderr, "%s: unknown property %s for ST\n", __func__, tl);
                    fclose(cf);
                    return 46;
                }

                break;

            case APP_CONF_MECT:
                if ((tl = app_property_name_check(l, "enabled"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.mect.enabled = atoi(l);
                    else
                    {
                        fclose(cf);
                        return 47;
                    }
                }
                else if ((tl = app_property_name_check(l, "debug"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.mect.debug = atoi(l);
                    else
                    {
                        fclose(cf);
                        return 48;
                    }
                }
                else if ((tl = app_property_name_check(l, "baud"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.mect.baud = atoi(l);
                    else
                    {
                        fclose(cf);
                        return 49;
                    }
                }
                else if (tl != NULL) {
                    fprintf(stderr, "%s: unknown property %s for MECT\n", __func__, tl);
                    fclose(cf);
                    return 50;
                }

                break;

            case APP_CONF_MB:
                if ((tl = app_property_name_check(l, "enabled"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.mb.enabled = atoi(l);
                    else
                    {
                        fclose(cf);
                        return 51;
                    }
                }
                else if ((tl = app_property_name_check(l, "ascii"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.mb.ascii = atoi(l);
                    else
                    {
                        fclose(cf);
                        return 52;
                    }
                }
                else if ((tl = app_property_name_check(l, "rtu"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.mb.rtu = atoi(l);
                    else
                    {
                        fclose(cf);
                        return 53;
                    }
                }
                else if ((tl = app_property_name_check(l, "baud"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.mb.baud = atoi(l);
                    else
                    {
                        fclose(cf);
                        return 54;
                    }
                }
                else if ((tl = app_property_name_check(l, "databits"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.mb.databits = atoi(l);
                    else
                    {
                        fclose(cf);
                        return 55;
                    }
                }
                else if ((tl = app_property_name_check(l, "parity"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.mb.parity = atoi(l);
                    else
                    {
                        fclose(cf);
                        return 56;
                    }
                }
                else if ((tl = app_property_name_check(l, "stopbits"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.mb.stopbits = atoi(l);
                    else
                    {
                        fclose(cf);
                        return 57;
                    }
                }
                else if ((tl = app_property_name_check(l, "ignore_echo"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.mb.ignore_echo = atoi(l);
                    else
                    {
                        fclose(cf);
                        return 58;
                    }
                }
                else if (tl != NULL) {
                    fprintf(stderr, "%s: unknown property %s for MB\n", __func__, tl);
                    fclose(cf);
                    return 59;
                }

                break;

            case APP_CONF_WD:
                if ((tl = app_property_name_check(l, "enabled"))) {
                    l = app_expect_equal(__func__, tl, ln, APP_CONFIG_FILE);
                    if (l != NULL)
                        app_conf.wd.enabled = atoi(l);
                    else
                    {
                        fclose(cf);
                        return 60;
                    }
                }
                else if (tl != NULL) {
                    fprintf(stderr, "%s: unknown property %s for WD\n", __func__, tl);
                    fclose(cf);
                    return 61;
                }

                break;

            case APP_CONF_NONE:
                /* Do nothing */
                break;

            default:
                fprintf(stderr, "%s: Unknown section %d in file %s\n", __func__, section, APP_CONFIG_FILE);
                fclose(cf);
                return 62;
                break;
            }
        }
    }
    fclose(cf);
    return 0;

#undef MAX_LINE_SIZE
}

/**
 * @brief Grab a version item
 *
 * @param char * item_name : item name;
 * @param char * item_pointer : poiter to the item;
 *
 * @return          NULL if error or pointer to item string
 *
 * @ingroup config
 */
char *app_version_item_get(char ** item_pointer, const char * item_name)
{
#define MAX_LINE_SIZE       81

    char line[MAX_LINE_SIZE];
    FILE *cf = NULL;
    int len = 0;

    cf = fopen(APP_CONFIG_VERSIONS_FILE, "r");
    if (cf == NULL) {
        perror(APP_CONFIG_VERSIONS_FILE);

        return NULL;
    }

    while ((fgets(line, MAX_LINE_SIZE, cf))!= NULL ) {

        if (strstr(line, item_name) != NULL)
            break;
    }
    fclose(cf);

    len = strlen(line);

    if (line[len - 1] == '\n')
        line[len - 1] = '\0';

    if (*item_pointer != NULL)
        free(*item_pointer);

    if (strchr(line, '=') != NULL)
    {
        *item_pointer = strdup(strchr(line, '=') + 1);
    }
    else
    {
        *item_pointer = strdup(line);
    }
    assert(*item_pointer != NULL);


    return *item_pointer;

#undef MAX_LINE_SIZE
}

int getMAC(char * mac)
{
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[1024];
    int success = 0;

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1) {
        return -1;
    };

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
        return -1;
    }

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (; it != end; ++it) {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
            if (! (ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                    success = 1;
                    break;
                }
            }
        }
        else {
            return -1;
        }
    }

    unsigned char mac_address[6]= "";

    if (success)
    {
        memcpy(mac_address, ifr.ifr_hwaddr.sa_data, 6);
        int j = 0;
        int i = 0;
        for (i = 0; i < 6; i++)
        {
            sprintf(&(mac[j]), "%02X:", mac_address[i]);
            j+=3;
        }
        mac[j-1]= '\0';
        printf("MAC:'%s'\n", mac);
    }
    else
    {
        return -1;
    }
    return 0;
}

int getIP(char * ip)
{
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;

    /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

    if (ioctl(fd, SIOCGIFADDR, &ifr) != 0)
    {
        return -1;
    }

    close(fd);

    /* display result */
    strcpy(ip, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
    printf("%s\n", ip);

    return 0;
}

char sd_mnt_point[256];
int SDCheck()
{
    return system("dmesg | grep -q mmcblk >/dev/null 2>&1");
}

int getSdCID(char * cid)
{
    cid[0]='\0';
    if(SDCheck() == 0)
    {
        /* get the CID info */
        FILE *fp;
        fp = fopen(CID_FILE, "r");

        if (fp != NULL)
        {
            /* Read the output a line at a time - output it. */
            if (fgets(cid, CID_LEN, fp) == NULL)
            {
				return -1;
            }
            /* close */
            fclose(fp);
            return (cid[0] == '\0');
        }
    }
    return -1;
}

int getSdSN(char * sn)
{
    char cid[CID_LEN];

    sn[0]='\0';
    if(SDCheck() == 0)
    {
        /* get the CID info */
        FILE *fp;
        fp = fopen(CID_FILE, "r");

        if (fp != NULL)
        {
            /* Read the output a line at a time - output it. */
            if (fgets(cid, CID_LEN, fp) != NULL)
            {
                /* get the SN of SD card */
                strncpy(sn, &cid[18], SN_LEN);
                sn[8] = '\0';
            }
            /* close */
            fclose(fp);
            return sn[0] == '\0';
        }
    }
    return -1;
}
