//Header file for threads.c

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "multicast.h"

pthread_t read_thread, write_thread;

void * write_messages();
void * read_messages();

