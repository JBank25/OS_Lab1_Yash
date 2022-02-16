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
#include <sys/stat.h>
#include <fcntl.h>

#define REDIRECTION_FOUND 1
#define REPLACE_STDIN 0
#define REPLACE_STDOUT 1
#define REPLACE_STDERR 2
#define STDIN 1
#define STDOUT 2
#define STDERR 4
#define FAIL_FILE_NOT_FOUND -1
#define NO_REDIRECTION 0
#define PLUS_OR_MINUS(A) (A == 1) ? printf(" + "): printf(" - ")
#define BUILTIN_BACKGROUND 1
#define BUILTIN_FOREGROUND 2
#define BUILTIN_JOBS 3
#define NOT_BUILTIN 0

void setProcess(process_t * process,char ** commands);
typedef enum jobStatus {STOPPED , RUNNING}status_t;

typedef struct process {
    char ** userInput;
    pid_t pid;
    char * inFile;
    char * outFile;
    char * errFile;
} process_t;

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

void handleProcess(process_t * process)
{
    int inputRedirectionFile;
    int outputRedirectionFile;
    int errRedirectionFile;

    printf("Input File: %s Output File: %s Err File: %s\n", process->inFile, process->outFile, process->errFile);
    int i = 0;
    /*printf("User arguements for process\n");
    while(process->userInput[i] != NULL)
    {
        printf("%d %s ",i ,process->userInput[i]);
        i++;
    }
    printf("\n");*/
    if(process->inFile != NULL)
    {
        inputRedirectionFile = open(process->inFile, O_RDONLY);
        dup2(inputRedirectionFile, STDIN_FILENO);
    }
    if(process->outFile != NULL)
    {
        outputRedirectionFile = open(process->outFile, O_WRONLY | O_CREAT | O_TRUNC, (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH));
        dup2(outputRedirectionFile, STDOUT_FILENO);
    }
    if(process->errFile != NULL)
    {
        errRedirectionFile = open(process->outFile, O_WRONLY | O_CREAT | O_TRUNC, (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH));
        dup2(errRedirectionFile, STDERR_FILENO);
    }
    execvp(process->userInput[0], process->userInput);
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

void handleBaseCase(char** userInput)
{
    printf("Inside basecase\n");
    int i = 0;
    while(userInput[i] != NULL)
    {
        printf("%s ", userInput[i]);
        i++;
    }
    execvp(userInput[0], userInput);

}

void clearProcesses(process_t * process1, process_t * process2)
{
    process1->errFile = NULL;
    process1->inFile = NULL;
    process1->outFile = NULL;

    process2->errFile = NULL;
    process2->inFile = NULL;
    process2->outFile = NULL;
    return;
}

void handleCommand(char** userInput)
{
    //we'll immediately check if this is a built in command, if so no job will be added
    int builtInCommandNumber = isBuiltInCommand(userInput);
    if(builtInCommandNumber)
    {
        handleBuiltin(builtInCommandNumber);
        return;
    }

    char ** commandsAfterPipe = NULL;
    char ** commandsBeforePipe = NULL;
    int jpid;
    int status;
    int pipeIndex = getPipeIndex(userInput);
    int redirectionFlag = redirectionCheck(userInput);
    job_t* newJob = (job_t*) malloc(sizeof(job_t));
    newJob = NULL;
    process_t *process1 = (process_t*) malloc(sizeof(process_t));
    process1->userInput = malloc(sizeof(char*)*100);
    process_t *process2 = (process_t*) malloc(sizeof(process_t));
    process2->userInput = malloc(sizeof(char*)*100);

    clearProcesses(process1, process2);//sets all strings in process structs to null
    //addJob();
    jpid = fork();// This is our job. We will launch our processes below!
    if(jpid == 0)
    {
        if((pipeIndex != -1))
        {
            printf("Pipe Exists\n");
            setPipeCommands(&commandsAfterPipe, &commandsBeforePipe, &userInput, pipeIndex);//gets the commands before and after pipe
            checkCommands(commandsBeforePipe, commandsAfterPipe);//just a check to ensure we have properly gotten the commands
            setProcess(process1, commandsBeforePipe);//This will assign values to the various files if we encounter a redirect
            setProcess(process2, commandsAfterPipe);
            handlePipe(process1, process2, jpid);
        }
        else if(redirectionFlag)
        {
            setProcess(process1, userInput);
            handleProcess(process1);
        }
        else
        {
            handleBaseCase(userInput);
        }
    }
    else
    {
        setpgid(jpid,0);
        tcsetpgrp (STDIN_FILENO, jpid);
        printf("Job id: %d JPID: %d\n", jpid, getpgid(jpid));
        //waitpid(jpid, &status, WUNTRACED); changed this while trying to implement pipes
        while(wait((int*) NULL) > 0);
        tcsetpgrp (STDIN_FILENO, getpid());
    }
}

void setProcess(process_t * process, char ** args)
{
    int scaler = sizeof(char*);
    char** commands = malloc(scaler);
    *commands = NULL;
    char * replaceStdin = "<";
    char * replaceStdout = ">";
    char * replaceStderr = "2>";
    int stdinCheck, stderrCheck, stdoutCheck, stdCheck;
    int counter = 0, sizeCounter = 1;
    int additionalCommands = 1;
    process->errFile = NULL;
    process->inFile = NULL;
    process->outFile = NULL;
    while(args[counter] != NULL)
    {
        stdinCheck = (strcmp(args[counter], replaceStdin) == 0);
        stdoutCheck = (strcmp(args[counter], replaceStdout) == 0) << 1;
        stderrCheck = (strcmp(args[counter], replaceStderr) == 0) << 2;
        stdCheck = stderrCheck | stdinCheck | stdoutCheck;
        //printf("Current arguements: %s\n", args[counter]);
        //printf("Stdcheck: %d\n", stdCheck);
        //printf("Incheck: %d outcheck: %d errcheck: %d\n", stdinCheck, stdoutCheck, stderrCheck);
        switch(stdCheck)
        {
            case STDIN:
                //printf("Stdin Check\n");
                additionalCommands = 0;
                process->inFile = args[counter+1];
                break;
            case STDOUT:
                //printf("Stdout Check\n");
                additionalCommands = 0;
                process->outFile = args[counter+1];
                //printf("Done with Stdin Check\n");
                break;
            case STDERR :
                //printf("Stderr Check\n");
                additionalCommands = 0;
                process->errFile = args[counter+1];
                break;
            default:
                if(additionalCommands)
                {
                    commands = (char**) realloc(commands, sizeof(char*) * (sizeCounter));
                    commands[counter] = args[counter];
                    sizeCounter++;
                }            
        }
        counter++;
    }
    process->userInput = commands;
}