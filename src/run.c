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
#include "threads.h"
#include "file_io.h"

void pthread_setup(struct read_info * r_i, char * addrs, int id, int num_processes){
	//Leader Process 0
	if (id == 0){
		if (pthread_create(&sequencer_thread, NULL, &sequencer, NULL)){
			printf("%d> Sequencer Thread error\n", id);		
		}
	}

    init_multicast(addrs, r_i, id, num_processes);
	if (pthread_create(&read_thread, NULL, &read_messages, NULL)){
		printf("%d> Read Thread error\n", id);
	}
	if (pthread_create(&write_thread, NULL, &write_messages, NULL)){
		printf("%d> Write Thread error\n", id);
	}
	pthread_join(read_thread, NULL);
	pthread_join(write_thread, NULL);
}

void print_status(char * addresses, int num_processes){
    printf("Num processes: %d\n", num_processes);
    printf("IP Addresses : ");
    int i = 0;
    for(i=0; i<num_processes; i++){
        printf("%s ", addresses+(i*16));
    }
    printf("\n---------------------------------\n");
}

struct read_info * set_up_read_info(int delay_time, int drop_rate, int listenfd, int ackfd, int sfd){
    struct read_info * r_i = (struct read_info *)malloc(sizeof(struct read_info));
    r_i -> delay_time = delay_time;
    r_i -> drop_rate  = drop_rate;
    r_i -> listenfd   = listenfd;
    r_i -> ackfd      = ackfd;
	r_i -> sfd        = sfd;
    return r_i;
}

int teardown(struct read_info * r_i, char * addresses, int listenfd, int ackfd, int sfd){
    close(listenfd);
    close(ackfd);
	close(sfd);
    free(r_i);
    free(addresses);
    close_multicast();
    return 1;
}

int main (int argc, const char* argv[]){
 
    int sfd, ackfd, listenfd, delay_time, drop_rate, id, num_processes;
    char * addresses;
    
    if(argc != 5){
        printf("chat usage: config_file delay_time drop_rate id\n");
        return -1;
    }
    else{
        addresses  = parse_config(argv[1], &num_processes);
        delay_time = atoi(argv[2]);
        drop_rate  = atoi(argv[3]);
        id         = atoi(argv[4]);
    }

    if(drop_rate < 0 || drop_rate > 100){
        printf("Drop rate must be between 0 and 100\n"); 
        return -1;
    }

    if(delay_time < 0){
        printf("Delay time must be positive\n");
        return -1;
    }

    srand(time(NULL));

    if(addresses == NULL) return -1;  //failed to read config file

    if(VERBOSE) print_status(addresses, num_processes);

    listenfd = set_up_listen(PORT+id, 0);		//socket for recieving messages
    ackfd    = set_up_listen(ACK_PORT+id, 1);	//socket for acks
	sfd      = set_up_listen(S_PORT+id, 0);		//special socket for proccess 0 sequencer/vector

    struct read_info * r_i = set_up_read_info(delay_time, drop_rate, listenfd, ackfd, sfd);
    pthread_setup(r_i, addresses, id, num_processes);
    teardown(r_i, addresses, listenfd, ackfd, sfd); 

    return 0;
}


