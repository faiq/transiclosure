#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <strings.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

//prints out the graph in the required format
void print_graph(int* g, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (g[j*n + i] == 1) {
                printf("%d %d\n", i+1, j+1);
            }
        }
    }
}

//print the graph like a matrix with rows and columns
void print_matrix(int* g, int n, int len) {
    for (int i = 0, j = 1; i < len; i++, j++) {
        printf("%d ", g[i]);
        if (j == n) {
            j = 0;
            printf("\n");
        }
    }
}

//given a k value and an area to work in,
//find the transitive_closure at that level
void transitive_closure(int* g, int k, int n, int work_start, int work_end) {
    for (int i = work_start; i < work_end; i++) {
        for (int j = 0; j < n; j++) {
            g[j*n + i] = g[j*n + i] || (g[k*n + i] && g[j*n + k]);
        }
    }
}

int main(int argc, char** argv) {
    //Get graph.in file
    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL) {
        perror("Could not open file");
        exit(EXIT_FAILURE);
    }

    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    int p, n;

    //read the first two lines (number of processes and nodes
    read = getline(&line, &len, fp);
    p = atoi(line);
    read = getline(&line, &len, fp);
    n = atoi(line);

    //graph is a square
    size_t graph_len = n*n;

    //make a shared memory key
    key_t shmkey = ftok("/dev/null", 5);

    //get shared memory from key (it will hold ints)
    int shmid = shmget(shmkey, graph_len * sizeof(int), 0644 | IPC_CREAT);
    if (shmid < 0) {
        perror("Could not create shared memory");
        exit(EXIT_FAILURE);
    }

    //attach the shared memory
    int* g = (int*)shmat(shmid, NULL, 0);
    if ((long int)g == -1) {
        perror("Could not attach shared memory");
        exit(EXIT_FAILURE);
    }

    //set the shared memory to zero
    bzero(g, graph_len*sizeof(int));

    //initialize the graph with starting values
    //One is subtracted because everything will
    //be zero indexed and the nodes start at one
    while ((read = getline(&line, &len, fp)) != -1) {
        int i = line[0] - '0' - 1;
        int j = line[2] - '0' - 1;
        g[j*n + i] = 1;
    }

    //clean up file
    fclose(fp);
    free(line);

    //actual transitive closure calculations
    for (int k = 0; k < n; k++) {

        //amount of work each process will do
        int work_size = n/p;

        //any work not divided evenly will go into it's own process
        int extra = n % p;

        //spawn a process for each batch of work
        for (int process = 0; process < p; process++) {

            //where to start the work for the process we are on 
            int work_start = process * work_size;

            //if in the child, do the work
            if (fork() == 0) {
                int work_end;
                //Determine if we have any extra work to
                //pass to the last process
                if (process == p - 1 && extra != 0)
                    work_end = extra;
                else
                    work_end = work_start + work_size;
                //do work
                transitive_closure(g, k, n, work_start, work_end);
                exit(EXIT_SUCCESS);
            }
        }

        //wait for all processes to finish
        for (int process = 0; process < p; process++) {
            wait(0);
        }
    }

    print_graph(g, n);

    return 0;
}
