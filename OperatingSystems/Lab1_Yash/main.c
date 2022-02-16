#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "file_redirection.h"
#include "pipe.h"
#include "jobs.h"

#define MAX_CHARS 2000
#define MAX_TOKENS 30

char ** parseInput(char * input);
void createJob(char * command, job_t * newJob, int cpid);
void initializeSignals();
void printJobs();
void setJobList(char * command, int cpid);
void setChildren(char** leftChild, char** rightChild, int pipeIndex, char** parsedCommand);
int main(int argc, char** argv)
{
    int scaler = sizeof(char*);
    char * cmd;
    char ** parsedCommand;
    int shellPid = getpid();
    setpgid(0, shellPid);
    tcsetpgrp (STDIN_FILENO, shellPid);
    printf("Shell PID %d Shell pgid: %d\n", shellPid, getpgid(shellPid));

    initializeSignals();
    initializeJobList();
    printf("Initial pid: %d\n", getpid());
    cmd = readline("# ");
    while(cmd)
    {
        int status;
        parsedCommand = parseInput(cmd);//parse commands
        handleCommand(parsedCommand);
        cmd = readline("# ");
    }
    free(parsedCommand);
}

void initializeSignals()
{
    signal(SIGTSTP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    return;
}

char ** parseInput(char * input)
{
    char * token = strtok(input, " ");
    int scaler = sizeof(char*);
    char ** hold = (char**)malloc(scaler);
    hold[0] = token;
    int i = 2;

    while(token!= NULL)
    {
        token = strtok(NULL, " ");
        hold = (char**) realloc(hold, i*scaler);
        hold[i-1] = token;
        i++;
    }
    return hold;
}