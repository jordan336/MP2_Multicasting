//Header file for threads.c

#include <stdio.h>
#include <stdlib.h>
#include "networking.h"
#include <pthread.h>

#define MAX_BUF_LEN 1000
#define PORT 15457

int ID, num_processes;
pthread_t read_thread, write_thread;

struct read_info {
    int delay_time;
    int drop_rate;
    int listenfd;
};

void * write_messages();
void * read_messages(void * listen);

