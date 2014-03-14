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
- Passing parsed ip_addresses all the way down to udp_send stuff
- Reliable unicast
- Basic multicast
- Reliable multicast
- Simulated delay
- Simulated loss

Change Log
==========

- 3/11 - Initial set up.  Old networking code + Makefile included
- 3/13 - Updated networking to fix freeaddrinfo() bug as well as add support beyond localhost 
- 3/13 - Support for talking between multiple processes.  Currently hardcoded to 2 processes and 2 messages. Run with ids 0 and 1.
- 3/13 - Improved formatting for reading / writing.  Processes continue indefinitely until quit is typed.
- 3/13 - Parsing of config file.  Variable number of processes. Need to finish this by making use of IP addresses in udp_send().

