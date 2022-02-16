#ifndef PIPES_H
#define PIPES_H

typedef struct process process_t;

int getPipeIndex(char ** inputs);
void handlePipe(process_t* process1, process_t* process2, int jpid);


#endif