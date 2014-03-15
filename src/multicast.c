//Multicast.c
//Reliable unicast, basic multicast, reliable multicast

#include "multicast.h"

char * addresses, * seen;
struct read_info * r_i;
int id, num_processes, num_seen;
char prefix[4]; 
pthread_mutex_t seen_mutex = PTHREAD_MUTEX_INITIALIZER;

void lock(){
    pthread_mutex_lock(&seen_mutex);
}

void unlock(){
    pthread_mutex_unlock(&seen_mutex);
}

int init_multicast(char * new_addrs, struct read_info * new_r_i, int new_id, int new_num_processes){
    addresses = new_addrs;
    r_i = new_r_i;
    id  = new_id;
    num_processes = new_num_processes;
    num_seen = 0;
    sprintf(prefix, "%d> ", new_id);
    seen = (char *)malloc(STORAGE * MAX_BUF_LEN * sizeof(char));  //set up array of previously seen messages
    memset(seen, 0, STORAGE * MAX_BUF_LEN * sizeof(char));
    return 1;
}

int close_multicast(){
    free(seen);
    return 1;
}

int previously_seen(char * message){
    lock();
    int i;
    for(i=0; i<num_seen; i++){
        if(strcmp(message, seen+(i*MAX_BUF_LEN*sizeof(char))) == 0){
            unlock();
            return 1;
        }
    }
    unlock();
    return 0;
}

int add_to_seen(char * message){
    lock();
    strncpy(seen+(num_seen*MAX_BUF_LEN), message, MAX_BUF_LEN);
    if(++num_seen >= STORAGE){
        num_seen = 0;
    }
    unlock();
    return 1;
}

int unicast_send(char * destination, int port, char * message){
    struct addrinfo *p;
    int talkfd = set_up_talk(destination, port, &p);
    
    if(talkfd != -1){
        udp_send(talkfd, message, p);
        //To Do: Implement reliable unicast by waiting here for an ACK or timeout and resending
        //wait_for_ack();
        freeaddrinfo(p);
        close(talkfd);
        return 1;
    }
    else{
        printf("unicast_send: cant send to ip: %s port: %d\n", destination, port);
        return -1;
    }
}

int unicast_receive(char * message){
    //add delay / drop
    return udp_listen(r_i->listenfd, message);
}

int b_multicast(char * message){
    int i;
    for(i=0; i<num_processes; i++){
        if(i == id) continue;
        unicast_send(addresses+(i*16), PORT+i, message);
    }
    return 1;
}

int r_multicast(char * message){
    strncpy(message, prefix, 3);
    add_to_seen(message);
    return b_multicast(message);
}

int r_deliver(char * message){
    int num_bytes = unicast_receive(message);
    if(num_bytes > 0 && !previously_seen(message)){
        //b_multicast(message);  //bug, wont work
        add_to_seen(message);
        printf("previously unseen, broadcasting\n");
        return num_bytes;
    }
    else if(num_bytes > 0){
        printf("previously seen\n");
    }
    return 0;
}



