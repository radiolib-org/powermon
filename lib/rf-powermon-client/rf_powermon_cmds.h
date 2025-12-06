#ifndef RF_POWERMON_CMDS_H
#define RF_POWERMON_CMDS_H

// the commands are based on TT7000 SCPI command set

#define RF_POWERMON_CMD_LINEFEED          "\n"
#define RF_POWERMON_RSP_LINEFEED          "\r\n"

#define RF_POWERMON_CMD_READ_POWER        "POWER:READ?"
#define RF_POWERMON_CMD_SYSTEM_EXIT       "SYS:EXIT"
#define RF_POWERMON_CMD_RESET             "*RST"
#define RF_POWERMON_CMD_ID                "*IDN?"

#endif
