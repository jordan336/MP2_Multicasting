//Threads.c
//Code executed by read and write threads

#include "threads.h"

void * write_messages(void * param){
    int i;
    char message[MAX_BUF_LEN];
    char prefix[4];  //3 + null
    struct addrinfo *p;
    char * addresses = (char *)param;

    printf("Enter your messages followed by return, quit to exit.\n");
    
    while(1){    
        fgets(message+3, MAX_BUF_LEN-3, stdin);
        if(strlen(message+3)>0 && message[strlen(message+3)-1+3] == '\n') message[strlen(message+3)-1+3]='\0';

        if(strcmp(message+3, "quit") == 0){
            pthread_cancel(read_thread);
            return 0;
        }

        sprintf(prefix, "%d> ", ID);
        strncpy(message, prefix, 3);

        for(i=0; i<num_processes; i++){
            if(i == ID) continue;
            int talkfd = set_up_talk(addresses+(i*16), PORT+i, &p);
            if(talkfd != -1){
                udp_send(talkfd, message, p);
                close(talkfd);
            }
            else{
                printf("Write thread: bad talkfd with ip: %s\n", addresses+(i*16));
            }
        }
    }

    return 0;
}

void * read_messages(void * param){
    struct read_info r_i = *((struct read_info *) param); 
    int listenfd = r_i.listenfd;
    //int delay_time = r_i.delay_time;
    //int drop_rate = r_i.drop_rate;
    char * buf = (char *)malloc(MAX_BUF_LEN * sizeof(char));

    while(1){
        int num_bytes = udp_listen(listenfd, buf);
        if(num_bytes > 0){
            printf("%s\n", buf);
        }
    }

    //free(buf);  //---TO DO: find out why the fuck this segfaults  -edit: i think you cant free from pthread
    return 0;
}

