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
int sequence;
int sequence_c[6];
int s_listen;

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
    seen = (char *)malloc(STORAGE * MAX_BUF_LEN * sizeof(char));  //set up array for buffered messages
    memset(seen, 0, STORAGE * MAX_BUF_LEN * sizeof(char));
    seq_nums = (int *)malloc(num_processes * sizeof(int));
	sequence = 0;
	if (id == 0)
		s_listen = set_up_listen(S_PORT-1, 0);
    int i;
    for(i=0; i<num_processes; i++) seq_nums[i] = 1;
	for(i=0; i<6; i++) sequence_c[i] = 0;
    return 1;
}

int close_multicast(){
    free(seen);
	close(s_listen);
    return 1;
}

void add_to_buffer(char * message){
	lock();
    memmove(seen+(num_seen*MAX_BUF_LEN), message, MAX_BUF_LEN);
	num_seen++;
    unlock();
}

void remove_from_buffer(int index){
	lock();
	memmove(seen+(index*MAX_BUF_LEN), seen+((index+1)*MAX_BUF_LEN), MAX_BUF_LEN*(num_seen-index-1));
	num_seen--;
	unlock();
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
                if(VERBOSE) printf("dropping ack and resending, ");
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
    if(VERBOSE) printf("timeout, ");
    fflush(stdout);
    return -1;  //resend
}

int send_ack(int dest_id, int seq_num){
    struct addrinfo *p;
    int talkfd = set_up_talk(addresses+(dest_id*16), ACK_PORT+dest_id, &p);
    char ack_message[4 + HEADER_SIZE];
    int random_delay = (r_i->delay_time == 0 ? 0 : rand() % (2 * r_i->delay_time));  //[0, 2*delay_time-1]

    if(VERBOSE) printf("send ack to: %d\n", dest_id);

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

int unicast_send(char * destination, int port, char * message, int s){
    char buf[MAX_BUF_LEN];
    *((int *)buf) = id;
	int i;
	// Get Sequence number from leader process (id = 0)
	if (ORDERING == 0){
		for (i=0; i<6; i++){
    		*((int *)buf+i+1) = s;//seq_nums[port-PORT];
		}
	}
	else{
		for (i=0; i<6; i++){
			*((int *)buf+i+1) = sequence_c[i];	
		}	
	}
	// Copy message over and begin sending
    memcpy(buf+HEADER_SIZE, message, MAX_BUF_LEN-HEADER_SIZE);
    struct addrinfo *p;
    int talkfd = set_up_talk(destination, port, &p);
    int ack_val = 0;

    if(VERBOSE) printf("sending to: %d .... ", port-PORT);
    fflush(stdout);

    if(talkfd != -1){
        do{
            //delay
            int random_delay = (r_i->delay_time == 0 ? 0 : rand() % (2 * r_i->delay_time));  //[0, 2*delay_time-1]
            if(random_delay > 0){
                usleep(random_delay*1000);
                if(VERBOSE) printf("delaying %d ms, ", random_delay);
                fflush(stdout);
            }
            udp_send(talkfd, buf, p);
        } while((ack_val = wait_for_ack(port-PORT)) == -1);
        
        if(VERBOSE) printf("received ack %d\n", ack_val);
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
    int seq_num[6];
	int i;
	for(i=0; i<6; i++){
		seq_num[i] = *((int *)message+i+1);
	}

    if(VERBOSE) printf("received from: %d seq_num: %d \n", sender, seq_num[sender]);

    //drop
    if(r_i->drop_rate > 0 && random_drop <= r_i->drop_rate){
        if(VERBOSE) printf("dropping packet\n");
        return 0;
    }

    if(bytes > 0){
        send_ack(sender, seq_num[sender]+1);
		add_to_buffer(message);
    }

	//Loop through queue and print messages that can be printed
	while(1)
	{
		int found = 0;

		int i, j, seq, from;
		int seq_c[6];
		char *m;
		for (i = 0; i < num_seen; i++)
		{
			m = seen+(i*MAX_BUF_LEN);
			from = *((int *)m);
			seq = *((int *)m+1);

			//Total Ordering
			if (ORDERING == TOTAL){
				if (seq == sequence+1)
				{
					//This message can be printed
					memmove(message, m+HEADER_SIZE, MAX_BUF_LEN-HEADER_SIZE);
					printf("%s\n", message);
					remove_from_buffer(i);
					found = 1;
					sequence = seq;
					break;
				}
			}
			//Casual Ordering
			else{
				for(j=0; j<6; j++){
					seq_c[j] = *((int *)m+j+1);
					//printf("%d",seq_c[j]);
				}
				//printf("\n");
				int printable = 1;
				for(j=0; j<6; j++){
					if (j == from){
						if(seq_c[j] != sequence_c[j]+1){
							printable = 0;
							break;
						}				
					}
					else if (seq_c[j] > sequence_c[j]){
						printable = 0;
						break;
					}
				}
				if (printable == 1){
					//This message can be printed
					memmove(message, m+HEADER_SIZE, MAX_BUF_LEN-HEADER_SIZE);
					printf("%s\n", message);
					remove_from_buffer(i);
					found = 1;
					sequence_c[from] = seq_c[from];
					break;
				}
			}
		}

		if (!found)
			break;
	}
 
    fflush(stdout);
    return bytes-HEADER_SIZE;
}

//////////////////////////////////////////////////////////////////////////////////
//Basic multicast

int b_multicast(char * message){
    int i;
	int s;
	s = get_sequencer();

	if (ORDERING == TOTAL){
		while ( s != sequence+1){}
		sequence = s;
	}
	else{
		sequence_c[id] = s;
	}

    for(i=0; i<num_processes; i++){
        if(i == id) continue;
        unicast_send(addresses+(i*16), PORT+i, message, s);
    }
    return 1;
}

//////////////////////////////////////////////////////////////////////////////////
//Sequencer

int get_sequencer(){
	char buf[MAX_BUF_LEN];
    *((int *)buf) = id;
	if (id == 0)
		*((int *)buf) = -1;

	struct addrinfo *p;
    int talkfd = set_up_talk(addresses, S_PORT, &p);
	udp_send(talkfd, buf, p);
	freeaddrinfo(p);
    close(talkfd);
	
	if (id == 0)
		udp_listen(s_listen, buf);
	else
		udp_listen(r_i->sfd, buf);

	return *((int *)buf);
}

int s_request(){
	char buf[MAX_BUF_LEN];
	udp_listen(r_i->sfd, buf);

	return *((int *)buf); //returns the id of requester
}

void s_send(int s, int sendTo){
	char buf[MAX_BUF_LEN];
	*((int *)buf) = s;
	struct addrinfo *p;
    int talkfd = set_up_talk(addresses, S_PORT+sendTo, &p);
	udp_send(talkfd, buf, p);
	freeaddrinfo(p);
    close(talkfd);
}

//////////////////////////////////////////////////////////////////////////////////
//Reliable multicast

int r_multicast(char * message){
    strncpy(message, prefix, 3);
    return b_multicast(message);
}

int r_deliver(char * message){
    return unicast_receive(message);
}

//////////////////////////////////////////////////////////////////////////////////


