/**
 * @file jobs.c
 * @author your name (you@domain.com)
 * @brief Jobs are in the foreground if we are waiting for them to terminate, meaning the 
 * shell is running them and the shell is blocked on a wait system call. When the job is 
 * finished terminal control will be restored. 
 * Job is in the background when the shell is not waiting on it. This can be done by using 
 * either & or by pressing Ctrl-Z.
 * fg and bg are two commands we can use as well which place the MOST RECENT job into the 
 * foreground or background. 
 *  
 * tcsetgrp(), setpgid(), and waitpid() are key to implementing job control.  
 * 
 * fg must send a SIGCONT to the most recent background or stopped process, print the
 * process to stdout, and wait for it to complete.
 * 
 * bg must send SIGCONT to most recent stopped process, print it to stdout, and NOT wait
 * for completion. 
 * @version 0.1
 * @date 2022-02-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "file_redirection.h"
#include "pipe.h"
#include "jobs.h"

#define PLUS_OR_MINUS(A) (A == 1) ? printf(" + "): printf(" - ")
#define BUILTIN_BACKGROUND 1
#define BUILTIN_FOREGROUND 2
#define BUILTIN_JOBS 3
#define NOT_BUILTIN 0

typedef enum jobStatus {STOPPED , RUNNING}status_t;

typedef struct process {
    char ** userInput;
    pid_t pid;
};
typedef struct job {
    char jobStr[80];//name of the job as it was run ./looper 1 & for example
    int pgid;//process group ID
    int isMostRecent;
    int indexInList;
    struct job * next;
    int redirectionFlag;
    int pipeFlag;
    int builtInFlag;
    int backgroundFlag;
    status_t status;//status of job stopped or running
} job_t;
job_t* jobList[20];//will be LL used to track all jobs
int backgroundCheck;

void initializeJobList()
{
    int i = 0;
    while(i<20)
    {
        jobList[i] = NULL;
        i++;
    }
}
void handleBackground()
{
    printf("Inside process background\n");
}

void handleForeground()
{
    printf("Inside process Foreground\n");
}

void handleJobs()
{
    printf("Inside process Jobs\n");
}

void handleBuiltin(int builtinValue)
{
    if(builtinValue == BUILTIN_BACKGROUND)
        handleBackground();
    else if(builtinValue == BUILTIN_FOREGROUND)
        handleForeground();
    else
        handleJobs();
}

int isBuiltInCommand(char** userInput)
{
    char * background = "bg";
    char* foreground = "fg";
    char *jobs = "jobs";
    //these if one of the checks is 1 and length is 1 then we have a built in
    int lengthCheck = sizeof(userInput)/sizeof(userInput[0]) == 1 ? 1:0;
    int backgroundCheck = strcmp(*userInput, background) == 0 ? 1:0;
    int foregroundCheck = strcmp(*userInput, foreground) == 0 ? 1:0;
    int jobsCheck = strcmp(*userInput, jobs) == 0 ? 1:0;

    backgroundCheck = backgroundCheck && lengthCheck;
    foregroundCheck = foregroundCheck && lengthCheck;
    jobsCheck = jobsCheck && lengthCheck;
    if(backgroundCheck)
        return BUILTIN_BACKGROUND;
    if(foregroundCheck)
        return BUILTIN_FOREGROUND;
    if(jobsCheck)
        return BUILTIN_JOBS;
    return NOT_BUILTIN;
}

void setPipeCommands(char*** commandsAfterPipe, char *** commandsBeforePipe, char *** userInput, int pipeIndex)
{
    printf("Inside pipe commands pipe index: %d\n", pipeIndex);
    (*commandsAfterPipe) = &(*userInput)[pipeIndex+1];
    (*commandsBeforePipe) = *userInput;
    (*commandsBeforePipe)[pipeIndex] = NULL;
}

void checkCommands(char** commandsBeforePipe, char** commandsAfterPipe)
{
    printf("Commands After pipe\n");
    while(*commandsAfterPipe!= NULL)
    {
        printf("%s\n", *commandsAfterPipe);
        commandsAfterPipe++;
    }
    printf("Commands Before pipe\n");
    while(*commandsBeforePipe!= NULL)
    {
        printf("%s\n", *commandsBeforePipe);
        commandsBeforePipe++;
    }
}


void isBackgroundThread(char** userInput)
{
    backgroundCheck = 0;
    int i = 0;
    while(userInput[i] != NULL)
        i++;
    i-=1;
    backgroundCheck = (strcmp("&", userInput[i]) == 0) ? 1 : 0;//if it is equal to & return a 1
}

void handleBaseCase(job_t * currentJob)
{
    printf("Inside basecase\n");
    int i = 0;
    while(userInput[i] != NULL)
    {
        printf("%s ", userInput[i]);
        i++;
    }
    printf("\n");
    int status;
    pid_t cpid = fork();
    if(cpid == 0)
    {
        if (redirectionCheck) {
            // handle
        }

        execvp(userInput[0], userInput);
    }
    else
    {
        setpgid(cpid, cpid);
        printf("Value of background check: %d Value of cpid: %d\n", backgroundCheck, cpid);
        if(backgroundCheck)
        {
            waitpid(cpid, &status, WNOHANG);
        }
        else
        {
            printf("Not giving terminal control\n");
            tcsetpgrp(0,cpid);
            waitpid(cpid, &status, WUNTRACED);
            printf("Returned status: %d\n", status);
            return;
        }
    }
}

void handleCommand(char** userInput)
{
    char ** commandsAfterPipe = NULL;
    char ** commandsBeforePipe = NULL;
    int cpid1, cpid2;
    int status;
    job_t* newJob = (job_t*) malloc(sizeof(job_t));

    newJob->builtInFlag = isBuiltInCommand(userInput);//check if the thread is a background thread, will have an & at the end if so
    newJob->pipeFlag = getPipeIndex(userInput);
    newJob->redirectionFlag = redirectionCheck(userInput);

    if(newJob->builtInFlag)
    {
        handleBuiltin(newJob->builtInFlag);
        return;
    }
    else if(newJob->pipeFlag!=-1)
    {
        printf("Inside pipe check if\n");
        setPipeCommands(&commandsAfterPipe, &commandsBeforePipe, &userInput, pipeCheck);
        checkCommands(commandsBeforePipe, commandsAfterPipe);
        handlePipe(commandsBeforePipe, commandsAfterPipe);
    }
    else {
        handleBaseCase(&newJob);
    }

    else if(checkRedirection)
    {
        printf("Inisde check redirection if\n");

        cpid = fork();
        if(cpid == 0)
            handleRedirection(userInput);
        else
        {
            setpgid(cpid, cpid);
            if(backgroundCheck == WUNTRACED)
                tcsetpgrp(0,cpid);
            waitpid(cpid, &status, backgroundCheck);
        }
    }
    else
    {
        handleBaseCase(userInput, cpid);
        printf("Returned from base case\n");
    }
}