/********************************
* Run.c       					*
*********************************
* Mark Kennedy: kenned31		*
* Jordan Ebel : ebel1			*
*********************************
* Description					*
*								*
* Code run by every process	    *
********************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "networking.h"
#include "multicast.h"
#include "threads.h"

#define VERBOSE 1

//char * config_file;
int delay_time, drop_rate, listenfd;
pthread_t read_thread, write_thread;

void pthread_setup(){
	if (pthread_create(&read_thread, NULL, &read_messages, &listenfd)){
		printf("%d> Read Thread error\n", ID);
	}
	if (pthread_create(&write_thread, NULL, &write_messages, NULL)){
		printf("%d> Write Thread error\n", ID);
	}
	pthread_join(read_thread, NULL);
	pthread_join(write_thread, NULL);
}

int main (int argc, const char* argv[]){
    
    if(argc != 5){
        printf("chat usage: config_file delay_time drop_rate id\n");
        return -1;
    }
    else{
        //config_file = argv[1];
        delay_time = atoi(argv[2]);
        drop_rate  = atoi(argv[3]);
        ID         = atoi(argv[4]);
    }

    listenfd = set_up_listen(PORT+ID);
    pthread_setup();
    close(listenfd);
    return 0;
}


