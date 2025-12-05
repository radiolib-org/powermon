#include "rf_powermon_client.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "rf_powermon_cmds.h"

static struct sockaddr_in server = {
  .sin_family = AF_INET,
  .sin_port = 0,
  .sin_addr = { 0 },
  .sin_zero = { 0 },
};

static int socket_read(int socket, char* cmd_buff) {
  int len = 0;
  do {
    ioctl(socket, FIONREAD, &len);
  } while(!len);

  len = recv(socket, cmd_buff, len, 0);
  if(len < 0) {
    // something went wrong
    return(0);
  }
  cmd_buff[len] = '\0';

  // strip trailing newline if present
  if((strlen(cmd_buff) > 1) && (cmd_buff[strlen(cmd_buff) - 1] == '\n')) {
    cmd_buff[strlen(cmd_buff) - 1] = '\0';
  }

  return(len);
}

static void socket_write(int socket, const char* data) {
  (void)send(socket, data, strlen(data), 0);
}

static int rf_powermon_exec(const char* cmd, char* rpl_buff) {
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(socket_fd < 0) {
    return(EXIT_FAILURE);
  }

  int ret = connect(socket_fd, (struct sockaddr*)&server, sizeof(struct sockaddr_in));
  if(ret) {
    return(EXIT_FAILURE);
  }
  
  socket_write(socket_fd, cmd);
  if(rpl_buff) {
    (void)socket_read(socket_fd, rpl_buff);
  }
  (void)close(socket_fd);
  return(EXIT_SUCCESS);
}

int rf_powermon_init(const char* hostname, int port) {
  struct hostent* hostnm = gethostbyname(hostname);
  if(!hostnm) {
    return(EXIT_FAILURE);
  }

  memcpy(&server.sin_addr, hostnm->h_addr_list[0], hostnm->h_length);
  server.sin_port = htons(port);
  return(EXIT_SUCCESS);
}

int rf_powermon_read(float* val) {
  char rpl_buff[256];
  int ret = rf_powermon_exec(RF_POWERMON_CMD_READ_POWER, rpl_buff);
  if(val) { *val = strtof(rpl_buff, NULL); }
  return(ret);
}

int rf_powermon_exit() {
  return(rf_powermon_exec(RF_POWERMON_CMD_SYSTEM_EXIT, NULL));
}

int rf_powermon_reset() {
  return(rf_powermon_exec(RF_POWERMON_CMD_RESET, NULL));
}
