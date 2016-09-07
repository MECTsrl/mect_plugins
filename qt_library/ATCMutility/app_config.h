#ifndef APP_CONFIG_H__
#define APP_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif
	/*
	 * Constant definitions
	 */
#define APP_CONFIG_DIR                  "/local/etc/sysconfig/"

#define APP_CONFIG_DIR_TEMP             "./"
#define APP_CONFIG_FILE                 APP_CONFIG_DIR "application.conf"
#define APP_CONFIG_BUILD_ID_FILE        APP_CONFIG_DIR ".fw_versions"
#define APP_CONFIG_SERIAL_FILE          "/etc/serial.conf"
#define APP_CONFIG_SERIAL_FILE2        APP_CONFIG_DIR "serial.conf"
#define APP_CONFIG_IPADDR_FILE	        APP_CONFIG_DIR "net.conf"
#define APP_CONFIG_MAC_FILE             "/etc/mac.conf"
#define APP_CONFIG_VERSIONS_FILE	        APP_CONFIG_DIR "versions.info"
#define	APP_CONFIG_LASTUPD_FILE		    APP_CONFIG_DIR "lastupdate.info"

#define DEFAULT_IPADDR0		"192.168.5.211"
#define	DEFAULT_NETMASK0	"255.255.255.0"
#define DEFAULT_GATEWAY0	"192.168.5.10"
#define DEFAULT_NAMESERVER01	""
#define DEFAULT_NAMESERVER02	""

	enum app_conf_canplc_e {
		APP_CONF_CANPLC_NONE = 0x0, /* No CANopen net selected */
		APP_CONF_CAN0PLC     = 0x1, /* CANopen net 0 selected */
		APP_CONF_CAN1PLC     = 0x2, /* CANopen net 1 selected */
		APP_CONF_CANALLPLC   = ~(APP_CONF_CAN0PLC | APP_CONF_CAN1PLC)
	};

	/*
	 * Type definition
	 */

	typedef struct app_conf_build_id_s {
		char *tag;
		char id[16];
	} app_conf_build_id_t;

	typedef struct app_conf_build_ids_s {
		app_conf_build_id_t main;
		app_conf_build_id_t cpld;
		app_conf_build_id_t kernel;
		app_conf_build_id_t timer;
		app_conf_build_id_t can;
		app_conf_build_id_t retentive;
		app_conf_build_id_t root;
		app_conf_build_id_t local;
		app_conf_build_id_t boot;
	} app_conf_build_ids_t;

	typedef struct app_conf_s {
		struct app_conf_irq0_s {    /* IRQ 0 settings */
			unsigned int outmask;   /* Digital outputs controlled */
			unsigned int encmask;   /* Encoders controlled */
			unsigned int anomask;   /* Analog output channels controlled */
			unsigned int enabled;   /* PLC thread is to be started? */
		} irq0;
		struct app_conf_irq1_s {    /* IRQ 0 settings */
			unsigned int outmask;   /* Digital outputs controlled */
			unsigned int encmask;   /* Encoders controlled */
			unsigned int anomask;   /* Analog output channels controlled */
			unsigned int enabled;   /* PLC thread is to be started? */
		} irq1;
		struct app_conf_plc0_s {    /* PLC 0 settings */
			char label[256];		/* PLC 0 name */
			int cycle;              /* Cycle time */
			int can;                /* Which CANopen network is attached */
			unsigned int outmask;   /* Digital outputs controlled */
			unsigned int encmask;   /* Encoders controlled */
			unsigned int anomask;   /* Analog output channels controlled */
		} plc0;
		struct app_conf_plc1_s {    /* PLC 1 settings */
			char label[256];		/* PLC 1 name */
			int cycle;              /* Cycle time */
			int can;                /* Which CANopen network is attached */
			unsigned int outmask;   /* Digital outputs controlled */
			unsigned int encmask;   /* Encoders controlled */
			unsigned int anomask;   /* Analog output channels controlled */
		} plc1;
		struct app_conf_plc2_s {    /* PLC 2 settings */
			char label[256];		/* PLC 2 name */
			int cycle;              /* Cycle time */
			int can;                /* Which CANopen network is attached */
			unsigned int outmask;   /* Digital outputs controlled */
			unsigned int encmask;   /* Encoders controlled */
			unsigned int anomask;   /* Analog output channels controlled */
		} plc2;
		struct app_conf_plc3_s {    /* PLC 3 settings */
			char label[256];		/* PLC 3 name */
			int cycle;              /* Cycle time */
			int can;                /* Which CANopen network is attached */
			unsigned int outmask;   /* Digital outputs controlled */
			unsigned int encmask;   /* Encoders controlled */
			unsigned int anomask;   /* Analog output channels controlled */
		} plc3;
		struct app_conf_hmi_s {     /* HMI settings */
			int cycle;              /* Cycle time */
			int can;                /* Which CANopen network is attached */
			unsigned int outmask;   /* Digital outputs controlled */
			unsigned int encmask;   /* Encoders controlled */
			unsigned int anomask;   /* Analog output channels controlled */
		} hmi;
		struct app_conf_st_s {      /* MECT serial test (slave) command interface settings */
			int enabled;            /* Is enabled? */
		} st;
		struct app_conf_mect_s {    /* MECT serial test (master) command interface settings */
			int enabled;            /* Is enabled? */
			int baud;		        /* Baud rate configuration*/
			int debug;              /* Activate debug code? */
		} mect;
		struct app_conf_mb_s {      /* MODBUS stack settings */
			int enabled;            /* Is enabled? */
			int ascii;              /* Use ASCII protocol */
			int rtu;		        /* Use RTU protocol */
			int baud;		        /* Baud rate configuration*/
			int databits;		    /* Databits bit configuration*/
			int parity;		        /* Parity bit configuration*/
			int stopbits;		    /* Stop bit configuration*/
			int ignore_echo;	    /* Echo configuration for RS485 half duplex*/		
		} mb;
		struct app_conf_wd_s {      /* Watch dog settings */
			int enabled;            /* Is enabled? */
		} wd;

		struct app_conf_fr_s {	/*Free run mode for at least one PLC*/
			int enabled;
		} free_run;	
	} app_conf_t;

	/*
	 * Global variabiles
	 */

	extern app_conf_t app_conf;

	/*
	 * Function prototypes
	 */

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
    char *app_version_item_get(char ** item_pointer, const char * item_name);

    /**
     * @brief Set a net conf item
     *
     * @param char * item_name : item name;
     * @param char * item : item value to set;
     *
     * @return          != 0 if error
     *
     * @ingroup config
     */
    int app_netconf_item_set(const char * item, const char * item_name);

    /**
	 * @brief Grab the serial number
	 *
	 * @return          NULL if error or pointer to serial number string
	 *
	 * @ingroup config
	 */
	char *app_build_serial_get(void);

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
    char *app_netconf_item_get(char ** item_pointer, const char * item_name);

    /**
	 * @brief Grab the MAC address
	 *
	 * @return          NULL if error or pointer to MAC string
	 *
	 * @ingroup config
	 */
    char *app_macconf_item_get(char ** item_pointer, const char * item_name);

	/**
	 * @brief Grab the ip address
	 *
     * @return          NULL if error or pointer to ip address string
	 *
	 * @ingroup config
	 */
	char *app_build_ipaddr_get(void);

    /**
     * @brief Grab the NetMask address
     *
     * @return          NULL if error or pointer to netmask string
     *
     * @ingroup config
     */
    char *app_build_netmask_get(void);

    /**
     * @brief Grab the Gateway address
     *
     * @return          NULL if error or pointer to gateway string
     *
     * @ingroup config
     */
    char *app_build_gateway_get(void);

    /**
     * @brief Grab the DNS 1 address
     *
     * @return          NULL if error or pointer to DNS 1 string
     *
     * @ingroup config
     */
    char *app_build_dns1_get(void);

    /**
     * @brief Grab the DNS 2 address
     *
     * @return          NULL if error or pointer to DNS 2 string
     *
     * @ingroup config
     */
    char *app_build_dns2_get(void);

    /**
	 * @brief Grab the build IDs for the main components of the system
	 * from the file filled at boot time.  The file format is:
	 * tag build_ID.
	 *
	 * @return          NULL if error or pointer to struct with build IDs
	 *
	 * @ingroup config
	 */
	app_conf_build_ids_t *app_build_ids_get(void);

	/**
	 * @brief Grab the last update
	 *
	 * @return          NULL if error or pointer to serial number string
	 *
	 * @ingroup config
	 */
	char * app_last_update_get(void);

	/**
	 * @brief Load the application configuration settings from the
	 * configuration file
	 *
	 * @return      0 for success; non-0 code for error
	 *
	 * @ingroup config
	 */
	int app_config_load(void);

#define CID_FILE "/sys/block/mmcblk0/device/cid"
#define CID_LEN 33
#define SN_LEN 8

    int getIP(const char * interface, char * ip);
    int getMAC(const char *interface, char * mac);
    int getSdCID(char * cid);
	int getSdSN(char * sn);
    int SDCheck();
    extern char sd_mnt_point[256];

#ifdef __cplusplus
}
#endif

#endif
