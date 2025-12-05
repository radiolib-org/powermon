#ifndef POWERMON_SOCKET_H
#define POWERMON_SOCKET_H

#ifdef __cplusplus
extern "C"{
#endif 

int rf_powermon_init(const char* hostname, int port);
int rf_powermon_read(float* val);
int rf_powermon_exit();
int rf_powermon_reset();

#ifdef __cplusplus
}
#endif

#endif
