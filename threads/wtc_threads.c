#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <errno.h>
#include <inttypes.h>
#include <unistd.h>
int threads; 
int verticies; 
int ** graph; 

inline uint64_t rdtsc() {
  unsigned int lo, hi;
  __asm__  __volatile__(
     "cpuid \n"
     "rdtsc" 
   : "=a"(lo), "=d"(hi) 
   :                    
   : "%ebx", "%ecx");     
  return ((((uint64_t)hi) << 32) | lo);
}

typedef struct work_args{
  int k; 
  int  row; 
  int  work_size; 
} work_args;

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
  fclose (input);
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
        graph[i][j] = graph[i][j] || (graph[i][k] && graph[k][j]);    
    } 
  }
  return NULL;
}

void transitive_closure () { 
  int k,i;
  pthread_t workers[threads]; 
  int max_work_size; 

  if (verticies % threads != 0) max_work_size = verticies/threads + 1; 
  else max_work_size = verticies/threads;  

  for (k = 0; k < verticies; k++) {
    int remaining_rows = verticies;     
    int row = 0; //what row the thread is starting on
    i = 0;
    while (remaining_rows > 0) {
      work_args * arg = malloc(sizeof (work_args)); 
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
      if (pthread_create(&workers[i], NULL , thread_ops, (void *) arg) == -1) 
        fprintf(stderr, "error in creation\n"); 
      remaining_rows -= work_size; 
      row = row + work_size;
      i++;
    } 
    for(i=0;i<threads;i++)
      pthread_join(workers[i],NULL); 
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
void clean_up(){ 
  int i; 
  for (i = 0; i < verticies; i++) 
    free(graph[i]);
  free(graph);
}  
int main (int argc, char ** argv) { 
  char * file = argv[1]; 
  if (strcmp (file, "graph.in") != 0) { 
    fprintf(stderr, "you passed in an incorrect file, please pass graph.in");  
    exit(EBADF);  
  }
  graph = parse_graph(file);
  printf("before\n");
  print_graph(graph);
  printf("\n");
  uint64_t start1 = rdtsc(); 
  transitive_closure();
  uint64_t stop1 = rdtsc();
  printf("after\n");
  print_graph(graph);
  unsigned int lo1 = start1 & (((uint64_t)2 << 32) - 1);
  unsigned int lo3 = stop1 & (((int64_t)2 << 32) - 1);
  int cycles1 = lo3 - lo1;
  float tz = ((float) (cycles1));
  float fact = 3092766000  * .000001;
  tz = tz / fact;
  printf("the time in milliseconds to do transitive closure with %d threads %f", threads, tz); 
  printf("\n");
  clean_up(); 
} 

