//File_io.c 
//Functions for reading from config file

#include "file_io.h"

FILE * open_file(const char * filename){
    FILE * file = fopen(filename, "r");
    if(file == NULL) printf("Could not open config file\n");
    return file;
}

void close_file(FILE * file){
    if(file != NULL){
        fclose(file);
    }
}

char * parse_config(const char * filename, int * num_processes){
    FILE * config_fd = open_file(filename);
    char * toRet, * toCopy;
    int i;

    if(config_fd != NULL){
        fscanf(config_fd, "%d ", num_processes);
        int size = *num_processes * sizeof(char) * 16;  //xxx.xxx.xxx.xxx\0
        toRet = (char *)malloc(size); 
        memset(toRet, 0, size);
        toCopy = toRet;

        for(i=0; i<*num_processes; i++){
            fscanf(config_fd, "%s\n", toCopy);
            toCopy += 16; 
        }

        close_file(config_fd);
        return toRet;
    }

    return NULL;
}

