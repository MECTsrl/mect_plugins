#include "app_logprint.h"

/**
 *
 * @brief severity level label
 *
 * @ingroup app_logprint
 *
 */
const char * _severity_[] =
{
    "",
    "###### Error: ",
    "Warning: ",
    "Info: ",
    "verbose: "
};

const char * _strerror_map_[] =
{
    "Success",
    "Format error",
    "Malformed data",
    "Generic error",
    "Missing data",
    "Permission denied"
};

int _error_code_;


