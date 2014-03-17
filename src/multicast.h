//Multicast header file

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "networking.h"
#include "types.h"

#define PORT 15457
#define ACK_PORT 25457
#define STORAGE 40
#define MAX_BUF_LEN 1000
#define HEADER_SIZE 2 * sizeof(int)

int init_multicast(char * new_addrs, struct read_info * new_r_i, int new_id, int new_num_processes);
int close_multicast();
int r_multicast(char * message);
int r_deliver(char * message);

