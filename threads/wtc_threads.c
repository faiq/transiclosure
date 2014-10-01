#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include "wtc_threads.h" 

int threads; 
int verticies; 
int ** graph; 
int ** prev_graph;

int ** parse_graph (char * file) { 
  int ** final;
  FILE * input  = fopen (file,"r"); 
  char buffer[1024];  
  if (input == NULL) perror ("Error opening file");  
  else {  
    int count = 1;
    while (fgets(buffer, sizeof(buffer)/sizeof(char), input)) {  
      if (count == 1) threads = atoi (buffer);  
      else if (count == 2) {
        verticies = atoi (buffer); 
        final = (int **) malloc (sizeof (int *) * verticies);
        int i; 
        for (i = 0; i < verticies; i++) {
          final[i] = malloc(sizeof (int) * verticies);
          memset(final[i], 0, sizeof (int) * verticies);
        } 
      } else {  
        char * pch = strtok (buffer," "); 
        int row = 0; 
        int column = 0; 
        int tok = 1;
        while (pch != NULL) {
          if  (tok == 1) row = atoi (pch); 
          else column = atoi (pch); 
          pch = strtok (NULL, " ,.-");
          tok++; 
        }
        if (row != 0 && column != 0) final[row - 1][column - 1] = 1;  
      } 
      count++; 
    } 
    int i, j; 
    for (i = 0; i < verticies; i++)
      for (j = 0; j < verticies; j++) 
        if (i == j) final[i][j] = 1;
  }
  return final;
}

void * thread_ops (void * args) { 
  work_args * arg = (work_args *) args;  
  int row = arg->row; 
  int work_size = arg->work_size; 
  int k = arg->k; 
  int i, j; 
  for (i = row; i < row+work_size; i++) {  
    for (j = 0; j < verticies; j++) {
      if (!prev_graph) { 
        graph[i][j] = graph[i][j];
      }
      else { 
        graph[i][j] = (prev_graph[i][j] || (prev_graph[i][k] && prev_graph[k][j]));    
      }
    } 
  }
  return NULL;
}

void transitive_closure () { 
  int k, j, i;
  pthread_t workers[threads]; 
  int max_work_size; 
  if (verticies % threads != 0) max_work_size = verticies/threads + 1; 
  else max_work_size = verticies/threads;  
  prev_graph = NULL; 
  for (k = 0; k < verticies; k++) {
    int remaining_rows = verticies;     
    int row = 0; //what row the thread is starting on
    i = 0;
    while (remaining_rows > 0) {
      work_args * arg = malloc (sizeof (work_args)); 
      int work_size;
      if (remaining_rows - max_work_size < 0) {
        int temp = remaining_rows - max_work_size;    
        work_size = max_work_size + temp;
      } else {
        work_size = max_work_size;
      }     
      arg->row = row; 
      arg->work_size = work_size; 
      arg->k = k;
      printf("row %d, work %d, k %d\n",row, work_size, k);  
      int thread = pthread_create(&workers[i], NULL , thread_ops, (void *) arg);
      remaining_rows -= max_work_size; 
      row = row + work_size;
      i++;
    } 
    for(i=0;i<threads;i++)
      pthread_join(workers[i],NULL); 
   
    if (!prev_graph) { 
      printf("\n");
      printf("prev is null k is %d\n", k);
      prev_graph = (int **) malloc (sizeof (int *) * verticies);
      int x; 
      for (x = 0; x < verticies; x++) {
        prev_graph[i] = malloc(sizeof (int) * verticies);
      } 
    } else { 
      printf("\n");
      printf("prev graph is not null k is %d\n",k);
      print_graph(prev_graph);
      printf("\n");
    } 
    memcpy (prev_graph, graph, ((sizeof (int *) * verticies) + (sizeof (int) * verticies)));
  }
  return;
}

void print_graph (int ** graph) { 
  int i, j; 
  for (i = 0; i < verticies; i++) {
    for (j = 0; j < verticies; j++) {
      printf("%d", graph[i][j]);  
    }
    printf("\n");
  }
}

int main (int argc, char ** argv) { 
  char * file = argv[1]; 
  graph = parse_graph(file);
  printf("before\n");
  print_graph(graph);
  printf("\n");
  transitive_closure();
  printf("after\n");
  print_graph(graph);
  printf("\n");
} 

