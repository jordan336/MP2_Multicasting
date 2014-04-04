//Threads.c
//Code executed by read and write threads

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
        int num_bytes = r_deliver(buf); 
        if(num_bytes > 0){
            //printf("%s\n", buf);
        }
    }

    //free(buf);  //---TO DO: find out why the f**k this segfaults  -edit: i think you cant free from pthread
    return 0;
}

//Process 0 is the leader process and will run this thread
void * sequencer(){

	int s = 1;
	int sendTo;

	while(1){
		sendTo = s_request();
		s_send(s, sendTo);
		s++;
	}

	return 0;
}


