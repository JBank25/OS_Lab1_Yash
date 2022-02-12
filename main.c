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

job_t ** jobList;//will be LL used to track all jobs
job_t * mostRecentJob;

int shell;      
pid_t shellPid;
char ** parseInput(char * input);
void createJob(char * command, job_t* newJob);
void printJobs();
void setJobList(job_t * newJob);

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
    job_t* firstJob = (job_t*)malloc(sizeof(job_t));
    strcpy(firstJob->jobStr , "shell");
    firstJob->status = RUNNING;
    firstJob->next = NULL;
    jobList = &firstJob;
    printf("Shell ID: %d Shell Group ID: %d\n",shellPid,getpgid(shellPid));
    while(cmd = readline("# "))
    {
        job_t *newJob = (job_t*) malloc(sizeof(job_t));
        createJob(cmd, newJob);
        setJobList(newJob);
        cpid = fork();
        if(cpid ==0)
        {
            printf("Forking\n");
            printJobs();
            exit(1);
            /*redirectionReturnCheck = 0;//redirectionCheck(parsedCommand);//see if any redirection commands exist
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
                execvp(parsedCommand[0], parsedCommand);*/
        }
        wait((int*) NULL);
    }

    free(parsedCommand);
    free(redirectCommands);
    free(jobList);
    free(mostRecentJob);
}

void printJobs()
{
    job_t **headJob = (job_t**) malloc(sizeof(job_t*));
    headJob = jobList;
    while(*jobList != NULL)
    {
        printf("Job: %s Status: %d Next: %p\n", (*jobList)->jobStr, (*jobList)->status, (*jobList)->next);
        *jobList = (*jobList)->next;
    }
    jobList = headJob;
}
void setJobList(job_t * newJob)
{
    printf("Setting job list for: %s address: %p\n", newJob->jobStr, newJob);
    job_t** jobHold = (job_t**) malloc(sizeof(job_t*));
    job_t* jobHold2 = (job_t*) malloc(sizeof(job_t));//had to add this line
    jobHold2 = *jobList;
    printf("Current Head: %s\n",(*jobList)->jobStr);
    jobHold = jobList;
    while((*jobList)->next != NULL)
    {
        printf("%s\n",(*jobList)->jobStr);
        (*jobList) = (*jobList)->next;
    }
    (*jobList)->next = newJob;
    jobList = jobHold;
    *jobList = jobHold2;//and this line to prevent ptr from only holding 2 values
    printf("New Head: %s\n",(*jobList)->jobStr);
}
void createJob(char * command, job_t * newJob)
{
    printf("Creating job for: %s\n", command);
    strcpy(newJob->jobStr, command);
    newJob->status = STOPPED;
    newJob->next = NULL;
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