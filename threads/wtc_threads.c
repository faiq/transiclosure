#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include "wtc_threads.h" 

int threads; 
int verticies; 
int ** graph; 

// function to return 1 or 0 depending whether or not they are connected
//  
int is_connected (int start, int interm, int stop) { 
  if (interm == 0) { 
    if (start == stop) return 1;
    else if (graph[start][stop]) { 
      return 1;       
    } else {
      return 0;  
    }  
  } else {
    return (is_connected (start, interm - 1, stop) || 
    ((is_connected (start, interm - 1, interm)) && (is_connected (interm, interm - 1, stop))));
  }
}
 
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

void transitive_closure () { 
  int k, j, i;
  for (k = 0; k < verticies; k++) 
    for (i = 0; i < verticies; i++) 
      for (j = 0; j < verticies; j++)
        graph[i][j] = is_connected(i, k, j); 
  return;
}

int main (int argc, char ** argv) { 
  char * file = argv[1]; 
  graph = parse_graph(file);
  transitive_closure();
  int i, j; 
  for (i = 0; i < verticies; i++)
    for (j = 0; j < verticies; j++) 
      printf("%d\n", graph[i][j]);  
} 

