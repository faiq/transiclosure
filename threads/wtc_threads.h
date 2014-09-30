#ifndef HEADER_FILE
#define HEADER_FILE
int ** parse_graph (char *); 
void print_graph (); 
void transitive_closure ();
typedef struct work_args{
  int k; 
  int  row; 
  int  work_size; 
} work_args;
#endif
