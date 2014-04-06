//Threads.c
//Code executed by read and write threads and sequencer

#include "threads.h"


void * write_messages(){
    char message[MAX_BUF_LEN];
    
    printf("Enter your messages followed by return, quit to exit.\n");
    
    while(1){    
        fgets(message+3, MAX_BUF_LEN-3-HEADER_SIZE, stdin);
        if(strlen(message+3)>0 && message[strlen(message+3)-1+3] == '\n') message[strlen(message+3)-1+3]='\0';

        if(strcmp(message+3, "quit") == 0){
            pthread_cancel(read_thread);
            return 0;
        }
    	
        r_multicast(message);
    }

    return 0;
}

void * read_messages(){
    char * buf = (char *)malloc(MAX_BUF_LEN * sizeof(char));

    while(1){
        r_deliver(buf); 
    }

    return 0;
}

//Process 0 is the leader process and will run this thread
void * sequencer(){

	int s = 1;
	int s_c[] = {1,1,1,1,1,1}; //vector timestamps for causal
	int sendTo, index;

	while(1){
		sendTo = s_request();
		if (ORDERING == TOTAL){
			s_send(s, sendTo);
			s++;
		}
		else if (ORDERING == CAUSAL){
			if (sendTo == -1)
				index = 0;
			else
				index = sendTo;
			//printf("%d\n",s_c[index]);
			s_send(s_c[index],sendTo);
			s_c[index]++;
		}
	}

	return 0;
}


