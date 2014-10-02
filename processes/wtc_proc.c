#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <strings.h>

int main(int argc, char** argv) {
    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL)
        perror("Could not open file");

    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    int p, n;
    size_t graph_len = n*n;

    read = getline(&line, &len, fp);
    p = atoi(line);
    read = getline(&line, &len, fp);
    n = atoi(line);

    key_t shmkey = ftok("/dev/null", 5);
    int shmid = shmget(shmkey, graph_len * sizeof(int), 0644 | IPC_CREAT);
    int* g = (int*)shmat(shmid, NULL, 0);
    bzero(g, graph_len*sizeof(int));

    while ((read = getline(&line, &len, fp)) != -1) {
        int i = line[0];
        int j = line[2];
        g[j*n + i] = 1;
    }

    return 0;
}
