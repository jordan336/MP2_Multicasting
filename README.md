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
- VERBOSE (defined in multicast.h) is a flag to display extra output
- ORDERING (defined in multicast.h) is a flag to determine the type of ordering used
    - 0 Total ordering
    - 1 Causal ordering

Config File
===========
format:
    num_processes \n
    ip_address 1 \n
    ip_address 2 \n 
    ...

