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

#define MAX_CHARS 2000
#define MAX_TOKENS 30

typedef enum jobStatus {STOPPED , RUNNING}status_t;
typedef struct job {
    char jobStr[80];//name of the job as it was run ./looper 1 & for example
    int pgid;//process group ID
    status_t status;//status of job stopped or running
    struct job * next;
} job_t;
job_t * jobList;//will be LL used to track all jobs
job_t * mostRecentJob;

int shell;      
pid_t shellPid;
char ** parseInput(char * input);
void createJob(char * parsedCommand);
void printJobs();

int main(int argc, char** argv)
{
    int scaler = sizeof(char*);
    pid_t cpid;
    char * cmd;
    char ** parsedCommand;
    char ** redirectCommands = (char**)malloc(scaler*3);
    int redirectionReturnCheck;
    int pipeIndex = 0;

    shellPid = getpid();//we'll initially get the process ID of our shell
    setpgid(shellPid,shellPid);//set the group ID of the shell
    job_t *headJob = malloc(sizeof(job_t));
    mostRecentJob = headJob;
    jobList = &headJob;
    headJob->pgid = shellPid;
    headJob->status = RUNNING;
    strcpy(headJob->jobStr, "SHELL");

    printf("Shell ID: %d Shell Group ID: %d\n",shellPid,getpgid(shellPid));
    while(cmd = readline("# "))
    {
        printf("Most recent Job: %s\n", mostRecentJob->jobStr);
        char inputHold[MAX_CHARS];
        strcpy(inputHold, cmd);
        parsedCommand = parseInput(cmd);//parse commands
        createJob(inputHold);
        pipeIndex = getPipeIndex(parsedCommand);//get index of the pipe if one exists
        cpid = fork();
        if(cpid ==0)
        {
            printJobs();
            redirectionReturnCheck = 0;//redirectionCheck(parsedCommand);//see if any redirection commands exist
            if(redirectionReturnCheck)
            {
                if(pipeIndex==0)
                    setRedirection(parsedCommand);
                else
                {
                    char ** commandsAfterPipe = &redirectCommands[pipeIndex];
                    char ** commandsBeforePipe = parsedCommand;
                    commandsAfterPipe[pipeIndex] = NULL;
                    setRedirection(commandsBeforePipe);
                    setRedirection(commandsAfterPipe);
                }
            }
            else
                execvp(parsedCommand[0], parsedCommand);
        }
        else
        {
            wait((int*) NULL);
        }
    }
    free(parsedCommand);
    free(redirectCommands);
    free(headJob);
}

void printJobs()
{
    job_t *headJob = jobList;
    while(jobList != NULL)
    {
        printf("Job: %s Status: %d\n", jobList->jobStr, jobList->status);
        jobList = jobList->next;
    }
    jobList = headJob;
}

void createJob(char * command)
{
    printf("Create Job for : %s\n", command);
    job_t *newJob = (job_t*) malloc(sizeof(job_t));
    strcpy(newJob->jobStr, command);
    newJob->status = STOPPED;
    mostRecentJob->next = &newJob;
    mostRecentJob = &newJob;
    mostRecentJob->next = NULL;
    printf("New Job: %s Job status: %d\n", newJob->jobStr, newJob->status);
    printf("sucess\n");
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