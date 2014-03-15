MP2_Multicasting
================

ECE 428 Distributed Systems MP2 - Multicasting

- Mark Kennedy - kenned31
- Jordan Ebel  - ebel1

Usage
=====
./chat config_file delay_time drop_rate id

Config File
===========
format:
    num_processes \n
    ip_address 1 \n
    ip_address 2 \n 
    ...

To Do
======
- Fix reliable multicast bug
- Reliable unicate, need to add ACKs
- Reliable causal ordering
- Reliable total ordering
- Simulated delay
- Simulated loss

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


