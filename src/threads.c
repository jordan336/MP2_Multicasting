//Threads.c
//Code executed by read and write threads

#include "threads.h"

//remove this when doing full multicasting
#define num_processes 2

void * write_messages(){
    int i;
    int num_messages = 0;
    char message[MAX_BUF_LEN];
    struct addrinfo *p;
    
    while(num_messages < 2){    
        printf("Enter your message: ");
        scanf("%s", message);
        num_messages++;

        for(i=0; i<num_processes; i++){
            if(i == ID) continue;
            int talkfd = set_up_talk(localhost, PORT+i, &p);
            if(talkfd != -1){
                udp_send(talkfd, message, p);
                close(talkfd);
            }
            else{
                printf("Write thread: bad talkfd\n");
            }
        }
    }

    return 0;
}

void * read_messages(void * listen){
    int listenfd = *((int *)listen);
    int num_messages = 0;
    char * buf = (char *)malloc(MAX_BUF_LEN * sizeof(char));

    while(num_messages < 2){
        int num_bytes = udp_listen(listenfd, buf);
        if(num_bytes > 0){
            num_messages++;
            printf("Received: %s\n", buf);
        }
    }

    close(listenfd);
    //free(buf);  //---TO DO: find out the fuck this segfaults
    return 0;    
}

