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
#include "file_io.h"

#define VERBOSE 1

int delay_time, drop_rate, listenfd;
char * addresses;

void pthread_setup(struct read_info * r_i, char * addrs){
	if (pthread_create(&read_thread, NULL, &read_messages, r_i)){
		printf("%d> Read Thread error\n", ID);
	}
	if (pthread_create(&write_thread, NULL, &write_messages, addrs)){
		printf("%d> Write Thread error\n", ID);
	}
	pthread_join(read_thread, NULL);
	pthread_join(write_thread, NULL);
}

void print_status(){
    printf("Num processes: %d\n", num_processes);
    printf("IP Addresses : ");
    int i = 0;
    for(i=0; i<num_processes; i++){
        printf("%s ", addresses+(i*16));
    }
    printf("\n---------------------------------\n");
}

struct read_info * set_up_read_info(int delay_time, int drop_rate, int listenfd){
    struct read_info * r_i = (struct read_info *)malloc(sizeof(struct read_info));
    r_i -> delay_time = delay_time;
    r_i -> drop_rate  = drop_rate;
    r_i -> listenfd   = listenfd;
    return r_i;
}

int main (int argc, const char* argv[]){
    
    if(argc != 5){
        printf("chat usage: config_file delay_time drop_rate id\n");
        return -1;
    }
    else{
        addresses  = parse_config(argv[1], &num_processes);
        delay_time = atoi(argv[2]);
        drop_rate  = atoi(argv[3]);
        ID         = atoi(argv[4]);
    }

    if(addresses == NULL) return -1;  //failed to read config file

    if(VERBOSE) print_status();

    listenfd = set_up_listen(PORT+ID);
    struct read_info * r_i = set_up_read_info(delay_time, drop_rate, listenfd);
    pthread_setup(r_i, addresses);
    close(listenfd);
    free(r_i);
    free(addresses);
    return 0;
}


