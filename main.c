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
void setChildren(char** leftChild, char** rightChild, int pipeIndex, char** parsedCommand);
int main(int argc, char** argv)
{
    int scaler = sizeof(char*);
    pid_t cpid;
    char * cmd;
    char ** parsedCommand;
    char ** redirectCommands = (char**)malloc(scaler*3);
    int redirectionReturnCheck;
    int pipeIndex = 0;

    while(cmd = readline("# "))
    {
        parsedCommand = parseInput(cmd);//parse commands
        pipeIndex = getPipeIndex(parsedCommand);//get index of the pipe if one exists
        cpid = fork();
        if(cpid ==0)
        {
            redirectionReturnCheck = redirectionCheck(parsedCommand);//see if any redirection commands exist
            if(redirectionReturnCheck)
            {
                if(pipeIndex==-1)
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
            else if(pipeIndex!=-1)
            {
                printf("No redirection\n");
                char ** commandsAfterPipe = &(parsedCommand[pipeIndex+1]);
                char ** commandsBeforePipe = &(parsedCommand[0]);
                commandsBeforePipe[pipeIndex] = NULL;
                /*int i = 0;
                printf("\n\n\n");
                printf("Commands before pipe:\n");
                while(commandsBeforePipe[i] != NULL)
                {
                    printf(" %s ", commandsBeforePipe[i]);
                    i++;
                }
                i = 0;
                printf("Commands after pips:\n");
                while(commandsAfterPipe[i] != NULL)
                {
                    printf(" %s ", commandsAfterPipe[i]);
                    i++;
                }        */        
                yashPipe(commandsBeforePipe, commandsAfterPipe);
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
}

void printJobs()
{
    job_t **headJob = (job_t**) malloc(sizeof(job_t*));
    headJob = jobList;
    while(*jobList != NULL)
    {
        printf("Job: %s Status: %d Next: %p PGID: %d\n", (*jobList)->jobStr, (*jobList)->status, (*jobList)->next, (*jobList)->pgid);
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
    newJob->pgid = getpid();
    strcpy(newJob->jobStr, command);
    newJob->status = STOPPED;
    newJob->next = NULL;
}

void setChildren(char** leftChild, char** rightChild, int pipeIndex, char** parsedCommand)
{
    printf("Setting children\n");
    int scaler = sizeof(char*);
    if(pipeIndex == -1)
    {
        printf("Setting values for left child\n");
        int i = 0;
        while(parsedCommand[i] != NULL)
        {
            leftChild = (char**) realloc(leftChild,scaler * (i+1));
            leftChild[i] = parsedCommand[i];
            i++;
        }
        i = 0;
        printf("Left Child: ");
        while(leftChild[i] != NULL)
        {
            printf("%s ", leftChild[i]);
            i++;
        }
        rightChild[0] = NULL;
        if(rightChild == NULL)
            printf("Right cHild: %s", rightChild[0]);
        return;
    }
    int i = 0;
    while(strcmp(parsedCommand[i], "|\0") != 0)
    {
        leftChild = (char**) realloc(leftChild,scaler * (i+1));
        leftChild[i] = parsedCommand[i];
        printf("parsed COmmand: %s child value: %s\n", leftChild[i], parsedCommand[i]);
        i++;
    }
    i++;
    int counter = 1;
    leftChild = (char**) realloc(leftChild,scaler * (i+1));
    leftChild[i] = NULL;
    while(parsedCommand[i] != NULL)
    {
        rightChild = (char**) realloc(rightChild,scaler * (counter));
        rightChild[counter-1] = parsedCommand[i];
        printf("parsed COmmand: %s child value: %s\n", rightChild[i], parsedCommand[i]);
        counter++;
        i++;
    }
    i = 0;
    printf("Left Child: ");
    while(leftChild[i] != NULL)
    {
        printf("%s ", leftChild[i]);
        i++;
    }
    i = 0;
    printf("\nRight Child: ");
    while(rightChild[i] != NULL)
    {
        printf("%s ", rightChild[i]);
        i++;
    }
    printf("\n");
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