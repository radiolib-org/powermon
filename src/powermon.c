#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <lgpio.h>
#include "argtable3/argtable3.h"

#ifndef GITREV
#define GITREV "unknown"
#endif

#define STR_HELPER(s) #s    
#define STR(s) STR_HELPER(s)     

#define PORT_DEFAULT          "/dev/ttyACM0"
#define SPEED_DEFAULT         115200
#define BUFF_SIZE             1024

static char rx_buff[BUFF_SIZE] = { 0 };
//static char tx_buff[BUFF_SIZE] = { 0 };

static struct conf_t {
  char port[64];
  int speed;
  int handle;
} conf = {
  .port = PORT_DEFAULT,
  .speed = SPEED_DEFAULT,
  .handle = -1,
};

enum sample_abs_level_unit {
  ABS_LEVEL_MICROWATT = 'u',
  ABS_LEVEL_MILLIWATT = 'm',
  ABS_LEVEL_WATT = 'w',
};

struct sample_t {
  float dbm;
  float abs_level;
  enum sample_abs_level_unit abs_unit;
};

static struct args_t {
  struct arg_str* port;
  struct arg_int* speed;
  struct arg_lit* help;
  struct arg_end* end;
} args;

static void sighandler(int signal) {
  (void)signal;
  exit(EXIT_SUCCESS);
}

static void exithandler(void) {
  int ret = lgSerialClose(conf.handle);
  if(ret < 0) {
    fprintf(stderr, "ERROR: Failed to close COM port (%s)\n", lguErrorText(ret));
  }
}

static char* strchr_first(char* s, const char* delims) {
  // by default, set the result to point at the final NULL in the input string
  // we know that this will never match any of our delimiters
  int len_str = strlen(s);
  char* ptr_first = s + len_str + 1;

  // grab the first delimiter and start iterating
  char* ptr;
  char delim = delims[0];
  while(delim) {
    ptr = strchr(s, delim);

    // if it was a hit, and is better than our current best, then save it
    if(ptr && (ptr < ptr_first)) {
      ptr_first = ptr;
    }

    delim = *delims++;
  }

  // if this was never changed, we did not find any of the delimiters
  if(ptr_first == s + len_str + 1) {
    return(NULL);
  }

  return(ptr_first);
}

static void parse_sample(char* sample_str, struct sample_t* sample) {
  if(!sample_str || (strlen(sample_str) != 10)) {
    fprintf(stderr, "Invalid sample: %s\n", sample_str);
    return;
  }

  // format is fixed-width
  char buff[8] = { 0 };

  // <1 char dbm sign><3 chars dbm level * 10>
  memcpy(buff, &sample_str[1], 3);
  int dbm_i = atoi(buff);
  if(sample_str[0] == '-') { dbm_i *= -1; }
  sample->dbm = (float)dbm_i / 10.0f;

  // <5 chars absolute level * 100><1 char absolute level unit>
  memcpy(buff, &sample_str[4], 5);
  sample->abs_level = (float)atoi(buff) / 100.0f;
  sample->abs_unit = sample_str[9];
}

static void process_rx(char* data, int len) {
  if(!data) {
    return;
  }

  // discard everything up to the first delimiter
  const char delims[] = "-+";
  char* ptr = data;
  char* ptr_prev = strchr_first(data, delims);
  if(!ptr_prev) {
    return;
  }
  
  char buff[16] = { 0 };
  while(ptr - data < len) {
    // find the next delimiter occurence
    ptr = strchr_first(ptr_prev + 1, delims);
    if(!ptr) { break; }

    // copy string between those two pointers
    memcpy(buff, ptr_prev, ptr - ptr_prev);

    struct sample_t sample = { 0 };
    parse_sample(buff, &sample);
    fprintf(stdout, "%.1f dBm\t %.2f %c\n", (double)sample.dbm, (double)sample.abs_level, sample.abs_unit);

    // clear the buffer and advance pointer
    memset(buff, 0, sizeof(buff));
    ptr_prev = ptr;
  }
}

static int run() {
  // open the port
  conf.handle = lgSerialOpen(conf.port, conf.speed, 0);
  if(conf.handle < 0) {
    fprintf(stderr, "ERROR: Failed to open COM port (%s)\n", lguErrorText(conf.handle));
    return(conf.handle);
  }

  // counters
  int av_count = 0;
  int rx_count = 0;

  // run until killed by the user
  for(;;) {
    // check how many bytes are available
    av_count = lgSerialDataAvailable(conf.handle);
    while(av_count) {
      // read up to the buffer size
      rx_count = lgSerialRead(conf.handle, rx_buff, av_count > BUFF_SIZE ? BUFF_SIZE : av_count);
      //fprintf(stdout, "read %d: %s\n", rx_count, rx_buff);

      process_rx(rx_buff, rx_count);

      // clear the buffer
      memset(rx_buff, 0, BUFF_SIZE);
      av_count -= rx_count;
    }
    
    // some small sleep to stop the thread for a bit
    usleep(1000);
  }

  return(0);
}

int main(int argc, char** argv) {
  void *argtable[] = {
    args.port = arg_str0("p", "port", NULL, "Which port to use, defaults to" STR(PORT_DEFAULT)),
    args.speed = arg_int0("s", "speed", NULL, "Port speed, defaults to " STR(SPEED_DEFAULT)),
    args.help = arg_lit0(NULL, "help", "Display this help and exit"),
    args.end = arg_end(2),
  };

  int exitcode = 0;
  if(arg_nullcheck(argtable) != 0) {
    fprintf(stderr, "%s: insufficient memory\n", argv[0]);
    exitcode = 1;
    goto exit;
  }

  int nerrors = arg_parse(argc, argv, argtable);
  if(args.help->count > 0) {
    fprintf(stdout, "RF power monitor, gitrev " GITREV "\n");
    fprintf(stdout, "Usage: %s", argv[0]);
    arg_print_syntax(stdout, argtable, "\n");
    fprintf(stdout, "After start, send SIGINT /Ctrl+C/ to stop\n");
    arg_print_glossary(stdout, argtable,"  %-25s %s\n");
    exitcode = 0;
    goto exit;
  }

  if(nerrors > 0) {
    /* Display the error details contained in the arg_end struct.*/
    arg_print_errors(stdout, args.end, argv[0]);
    fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
    exitcode = 1;
    goto exit;
  }

  atexit(exithandler);
  signal(SIGINT, sighandler);

  if(args.port->count) { strncpy(conf.port, args.port->sval[0], strlen(args.port->sval[0])); }
  if(args.speed->count) { conf.speed = args.speed->ival[0]; }

  exitcode = run();

exit:
  arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));

  return exitcode;
}