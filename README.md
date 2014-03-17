MP2_Multicasting
================

ECE 428 Distributed Systems MP2 - Multicasting

- Mark Kennedy - kenned31
- Jordan Ebel  - ebel1

Usage
=====
- ./chat config_file delay_time drop_rate id
- ids must be [0, num_processes-1]
- delay time in milliseconds, must be positive
- drop_rate must be [0, 100]
- must have all ids present in chatroom or infinite timeouts...

Config File
===========
format:
    num_processes \n
    ip_address 1 \n
    ip_address 2 \n 
    ...

To Do
======
- Put in ACK drops and timeout handling with sequence numbers
- Completely remove reliable multicast probably, its not unnecessary with full reliable unicast
- Reliable causal ordering
- Reliable total ordering

Known bugs
==========
- If delay rates too high, timeout is too fast.  Without sequence numbers, everything gets messed up.
- If same message typed twice, will only be sent once because it appears as an already seen message in reliable multicast.  Reliable multicast specifically looks for previously seen messages to terminate broadcasts.


Change Log
==========

- 3/11 - Initial set up.  Old networking code + Makefile included
- 3/13 - Updated networking to fix freeaddrinfo() bug as well as add support beyond localhost 
- 3/13 - Support for talking between multiple processes.  Currently hardcoded to 2 processes and 2 messages. Run with ids 0 and 1.
- 3/13 - Improved formatting for reading / writing.  Processes continue indefinitely until quit is typed.
- 3/13 - Parsing of config file.  Variable number of processes. Need to finish this by making use of IP addresses in udp_send().
- 3/14 - Correctly using IP addresses to send.
- 3/14 - Unicast working, not reliable.  Need to add ACKs to support reliable unicast
- 3/14 - Reliable multicast close.  Bug wont allow read thread to broadcast
- 3/15 - Reliable multicast seems to be fixed.  Check for strange / failed behavior on broadcast after receiving.
- 3/15 - Reliable unicast with ACKs working.  Reliable multicast using ACKs working.
- 3/16 - Random delay and drop working
- 3/17 - Added sequence numbers for every message

