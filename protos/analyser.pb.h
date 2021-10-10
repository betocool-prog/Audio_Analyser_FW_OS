/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.4 */

#ifndef PB_ANALYSER_PB_H_INCLUDED
#define PB_ANALYSER_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Enum definitions */
typedef enum _MESSAGE_TYPE {
    MESSAGE_TYPE_SET_MESSAGE = 0,
    MESSAGE_TYPE_GET_MESSAGE = 1,
    MESSAGE_TYPE_ACTION_MESSAGE = 2
} MESSAGE_TYPE;

typedef enum _MODE {
    MODE_FREE_RUNNING = 0,
    MODE_PULSED = 1
} MODE;

typedef enum _FUNCTION_TYPE {
    FUNCTION_TYPE_SINE = 0,
    FUNCTION_TYPE_IMPULSE = 1
} FUNCTION_TYPE;

/* Struct definitions */
typedef struct _Config {
    char dummy_field;
} Config;

typedef struct _Status {
    char dummy_field;
} Status;

typedef struct _Command {
    bool has_reset;
    bool reset;
} Command;

typedef struct _SignalConfig {
    bool has_function;
    FUNCTION_TYPE function;
    bool has_mode;
    MODE mode;
    bool has_frequency;
    uint32_t frequency;
    bool has_amplitude;
    float amplitude;
} SignalConfig;

typedef struct _Service {
    uint32_t xfer_id;
    MESSAGE_TYPE message_type;
    bool has_status;
    Status status;
    bool has_config;
    Config config;
    bool has_signalconfig;
    SignalConfig signalconfig;
    bool has_command;
    Command command;
} Service;


/* Helper constants for enums */
#define _MESSAGE_TYPE_MIN MESSAGE_TYPE_SET_MESSAGE
#define _MESSAGE_TYPE_MAX MESSAGE_TYPE_ACTION_MESSAGE
#define _MESSAGE_TYPE_ARRAYSIZE ((MESSAGE_TYPE)(MESSAGE_TYPE_ACTION_MESSAGE+1))

#define _MODE_MIN MODE_FREE_RUNNING
#define _MODE_MAX MODE_PULSED
#define _MODE_ARRAYSIZE ((MODE)(MODE_PULSED+1))

#define _FUNCTION_TYPE_MIN FUNCTION_TYPE_SINE
#define _FUNCTION_TYPE_MAX FUNCTION_TYPE_IMPULSE
#define _FUNCTION_TYPE_ARRAYSIZE ((FUNCTION_TYPE)(FUNCTION_TYPE_IMPULSE+1))


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define Status_init_default                      {0}
#define Config_init_default                      {0}
#define SignalConfig_init_default                {false, _FUNCTION_TYPE_MIN, false, _MODE_MIN, false, 0, false, 0}
#define Command_init_default                     {false, 0}
#define Service_init_default                     {0, _MESSAGE_TYPE_MIN, false, Status_init_default, false, Config_init_default, false, SignalConfig_init_default, false, Command_init_default}
#define Status_init_zero                         {0}
#define Config_init_zero                         {0}
#define SignalConfig_init_zero                   {false, _FUNCTION_TYPE_MIN, false, _MODE_MIN, false, 0, false, 0}
#define Command_init_zero                        {false, 0}
#define Service_init_zero                        {0, _MESSAGE_TYPE_MIN, false, Status_init_zero, false, Config_init_zero, false, SignalConfig_init_zero, false, Command_init_zero}

/* Field tags (for use in manual encoding/decoding) */
#define Command_reset_tag                        1
#define SignalConfig_function_tag                1
#define SignalConfig_mode_tag                    2
#define SignalConfig_frequency_tag               6
#define SignalConfig_amplitude_tag               7
#define Service_xfer_id_tag                      1
#define Service_message_type_tag                 2
#define Service_status_tag                       3
#define Service_config_tag                       4
#define Service_signalconfig_tag                 5
#define Service_command_tag                      6

/* Struct field encoding specification for nanopb */
#define Status_FIELDLIST(X, a) \

#define Status_CALLBACK NULL
#define Status_DEFAULT NULL

#define Config_FIELDLIST(X, a) \

#define Config_CALLBACK NULL
#define Config_DEFAULT NULL

#define SignalConfig_FIELDLIST(X, a) \
X(a, STATIC,   OPTIONAL, UENUM,    function,          1) \
X(a, STATIC,   OPTIONAL, UENUM,    mode,              2) \
X(a, STATIC,   OPTIONAL, UINT32,   frequency,         6) \
X(a, STATIC,   OPTIONAL, FLOAT,    amplitude,         7)
#define SignalConfig_CALLBACK NULL
#define SignalConfig_DEFAULT NULL

#define Command_FIELDLIST(X, a) \
X(a, STATIC,   OPTIONAL, BOOL,     reset,             1)
#define Command_CALLBACK NULL
#define Command_DEFAULT NULL

#define Service_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, UINT32,   xfer_id,           1) \
X(a, STATIC,   REQUIRED, UENUM,    message_type,      2) \
X(a, STATIC,   OPTIONAL, MESSAGE,  status,            3) \
X(a, STATIC,   OPTIONAL, MESSAGE,  config,            4) \
X(a, STATIC,   OPTIONAL, MESSAGE,  signalconfig,      5) \
X(a, STATIC,   OPTIONAL, MESSAGE,  command,           6)
#define Service_CALLBACK NULL
#define Service_DEFAULT NULL
#define Service_status_MSGTYPE Status
#define Service_config_MSGTYPE Config
#define Service_signalconfig_MSGTYPE SignalConfig
#define Service_command_MSGTYPE Command

extern const pb_msgdesc_t Status_msg;
extern const pb_msgdesc_t Config_msg;
extern const pb_msgdesc_t SignalConfig_msg;
extern const pb_msgdesc_t Command_msg;
extern const pb_msgdesc_t Service_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define Status_fields &Status_msg
#define Config_fields &Config_msg
#define SignalConfig_fields &SignalConfig_msg
#define Command_fields &Command_msg
#define Service_fields &Service_msg

/* Maximum encoded size of messages (where known) */
#define Status_size                              0
#define Config_size                              0
#define SignalConfig_size                        15
#define Command_size                             2
#define Service_size                             33

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
