//Multicast.c
//Reliable unicast, basic multicast, reliable multicast

#include "multicast.h"

char * addresses, * seen;
struct read_info * r_i;
int id, num_processes, num_seen;
char prefix[4];
pthread_mutex_t seen_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t broadcast_thread;
int * seq_nums;

//////////////////////////////////////////////////////////////////////////////////
//Utilities

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
    seq_nums = (int *)malloc(num_processes * sizeof(int));
    int i;
    for(i=0; i<num_processes; i++) seq_nums[i] = 1;
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

//////////////////////////////////////////////////////////////////////////////////
//ACKs

int wait_for_ack(int expected_sender){
    int count = 0;
    char reply[MAX_BUF_LEN];

    while(count++ < 10000000){
        if(udp_listen(r_i->ackfd, reply) > 0){
            int random_drop  = rand() % 101;  //[0, 100]

            //drop
            if(r_i->drop_rate > 0 && random_drop <= r_i->drop_rate){
                printf("dropping ack and resending, ");
                fflush(stdout);
                return -1;
            }

            int sender = *((int *)reply);
            int seq_num = atoi(reply+HEADER_SIZE);
            if(sender != expected_sender) return -1;  //ignore old ACKs from previous connections
            if(seq_num > seq_nums[sender]) seq_nums[sender] = seq_num;  //ignore old ACKs from current connection
            return seq_num; //got current ack
        }
    }
    printf("timeout, ");
    fflush(stdout);
    return -1;  //resend
}

int send_ack(int dest_id, int seq_num){
    struct addrinfo *p;
    int talkfd = set_up_talk(addresses+(dest_id*16), ACK_PORT+dest_id, &p);
    char ack_message[4 + HEADER_SIZE];
    int random_delay = (r_i->delay_time == 0 ? 0 : rand() % (2 * r_i->delay_time));  //[0, 2*delay_time-1]

    //printf("send ack to: %d\n", dest_id);

    if(talkfd != -1){
        *((int *)ack_message) = id;
        sprintf(ack_message+HEADER_SIZE, "%d", seq_num);
        if(random_delay > 0) usleep(random_delay*1000); //delay
        udp_send(talkfd, ack_message, p);
        freeaddrinfo(p);
        close(talkfd);
        return 1;
    }
    printf("Unable to send ACK to process: %d\n", dest_id);
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////
//Unicast

int unicast_send(char * destination, int port, char * message){
    char buf[MAX_BUF_LEN];
    *((int *)buf) = id;
    *((int *)buf+1) = seq_nums[port-PORT];
    memcpy(buf+HEADER_SIZE, message, MAX_BUF_LEN-HEADER_SIZE);
    struct addrinfo *p;
    int talkfd = set_up_talk(destination, port, &p);
    int ack_val = 0;

    printf("sending to: %d .... ", port-PORT);
    fflush(stdout);

    if(talkfd != -1){
        do{
            //delay
            int random_delay = (r_i->delay_time == 0 ? 0 : rand() % (2 * r_i->delay_time));  //[0, 2*delay_time-1]
            if(random_delay > 0){
                usleep(random_delay*1000);
                printf("delaying %d ms, ", random_delay);
                fflush(stdout);
            }
            udp_send(talkfd, buf, p);
        } while((ack_val = wait_for_ack(port-PORT)) == -1);
        
        printf("received ack %d\n", ack_val);
        fflush(stdout);

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
    int random_drop  = rand() % 101;  //[0, 100]
    int bytes = udp_listen(r_i->listenfd, message);
    int sender  = *((int *)message);
    int seq_num = *((int *)message+1);

    printf("received from: %d seq_num: %d \n", sender, seq_num);

    //drop
    if(r_i->drop_rate > 0 && random_drop <= r_i->drop_rate){
        printf("dropping packet\n");
        return 0;
    }

    if(bytes > 0){
        send_ack(sender, seq_num+1);
        if(seq_num < seq_nums[sender]){
            printf("retransmission\n");
            return 0;
        }
        seq_nums[sender] = seq_num+1;
        memmove(message, message+HEADER_SIZE, MAX_BUF_LEN-HEADER_SIZE); //remove buffer
    }
 
    fflush(stdout);
    return bytes-HEADER_SIZE;
}

//////////////////////////////////////////////////////////////////////////////////
//Basic multicast

int b_multicast(char * message){
    int i;
    for(i=0; i<num_processes; i++){
        if(i == id) continue;
        unicast_send(addresses+(i*16), PORT+i, message);
    }
    return 1;
}

//////////////////////////////////////////////////////////////////////////////////
//Broadcast thread 

void * broadcast_message(void * param){
    b_multicast((char *)param);
    return 0;
}

int start_broadcast_thread(char * message){
    if (pthread_create(&broadcast_thread, NULL, &broadcast_message, message)){
		printf("%d> Broadcast Thread error\n", id);
	}
	//pthread_join(broadcast_thread, NULL);
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////
//Reliable multicast

int r_multicast(char * message){
    strncpy(message, prefix, 3);
    //add_to_seen(message);
    return b_multicast(message);
}

int r_deliver(char * message){
    return unicast_receive(message);
    /* int num_bytes = unicast_receive(message);
    if(num_bytes > 0 && !previously_seen(message)){
        printf("broadcasting\n");
        start_broadcast_thread(message);
        add_to_seen(message);
        return num_bytes;
    }
    else if(num_bytes > 0){
        printf("previously seen\n");
    }
    return 0; */
}

//////////////////////////////////////////////////////////////////////////////////


