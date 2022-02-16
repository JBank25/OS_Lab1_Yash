#ifndef JOBS_H
#define JOBS_H

#define BUILTIN_BACKGROUND 1
#define BUILTIN_FOREGROUND 2
#define BUILTIN_JOBS 3
#define NOT_BUILTIN 0
typedef struct process process_t;
typedef struct job job_t;

void initializeJobList();
void handleBackground();
void handleForeground();
void handleJobs();
void handleBuiltin(int builtinValue);
int isBuiltInCommand(char** userInput);
void isBackgroundThread(char** userInput);
void handleBaseCase(char** userInput);
void cleaProcesses(process_t * process1, process_t * process2);
void handleCommand(char** userInput);
void setProcess(process_t * process, char ** args);
void handleProcess(process_t * process);
void setPipeCommands(char*** commandsAfterPipe, char *** commandsBeforePipe, char *** userInput, int pipeIndex);
void checkCommands(char** commandsBeforePipe, char** commandsAfterPipe);
#endif