#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>

int main(int argc, char** argv) {
    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL)
        perror("Could not open file");

    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    int p, n;

    read = getline(&line, &len, fp);
    p = atoi(line);
    read = getline(&line, &len, fp);
    n = atoi(line);

    key_t shmkey = ftok("/dev/null", 5);
    int shmid = shmget(shmkey, n * n + sizeof(int) * p, 0644 | IPC_CREAT);

    while ((read = getline(&line, &len, fp)) != -1) {
    }

    return 0;
}
