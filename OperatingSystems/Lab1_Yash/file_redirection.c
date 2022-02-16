/**
 * @file file_redirection.c
 * @author Joshua Urbank
 * @brief Pipe creates unidirectional communication between two related processes.
 * pipe(pipfd) will return an array with index 0 = write end of pip index 1 = read end of 
 * pipe. Dup syscall duplicates file descriptors. Useful to copies file desrcriptors from
 * one place to another. Dup2 takes an old file descritpro and a new one. Closes the new file
 * descriptor if it is open. Then it copies the file descriptor tables entries for the
 * old file descriptor to the new file descriptor. dup2(fd,1) would close entry 1 in the
 * file descriptor table and copy the pointer that fd points to into index 1. 
 * @version 0.1
 * @date 2022-02-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "jobs.h"

#define REDIRECTION_FOUND 1
#define REPLACE_STDIN 0
#define REPLACE_STDOUT 1
#define REPLACE_STDERR 2
#define STDIN 1
#define STDOUT 2
#define STDERR 4
#define FAIL_FILE_NOT_FOUND -1
#define NO_REDIRECTION 0
#define RESET_FLAGS(A,B,C,D) (A*B*C*D*0)

//This program acting under the assumption that there will not be any < > or 2> 
//passed in as the first arguement. Might need to error check for the last arguement
//not being NULL inside of the if statements
int redirectionCheck(char** commands)
{
    char *redirectSymbol0 = "<";
    char *redirectSymbol1 =  ">";
    char *redirectSymbol2 =  "2>";
    int counter = 0;
    int redirectCheck0, redirectCheck1, redirectCheck2;

    while(commands[counter] != NULL)
    {
        redirectCheck0 = strcmp(redirectSymbol0, commands[counter]) == 0;
        redirectCheck1 = strcmp(redirectSymbol1, commands[counter]) == 0;
        redirectCheck2 = strcmp(redirectSymbol2, commands[counter]) == 0;
        if(redirectCheck0 || redirectCheck1 || redirectCheck2)
        {
            return REDIRECTION_FOUND;
        }
        counter+=1;
    }
    return NO_REDIRECTION;
}

void handleRedirection(char **redirectArgs)
{
    int scaler = sizeof(char*);
    char** commands = malloc(scaler);
    *commands = NULL;
    char * replaceStdin = "<";
    char * replaceStdout = ">";
    char * replaceStderr = "2>";
    int stdinCheck, stderrCheck, stdoutCheck, stdCheck;
    int inputRedirectionFile, outputRedirectionFile;
    int counter = 0, sizeCounter = 1;
    int additionalCommands = 1;
    while(redirectArgs[counter] != NULL)
    {
        stdinCheck = (strcmp(redirectArgs[counter], replaceStdin) == 0);
        stdoutCheck = (strcmp(redirectArgs[counter], replaceStdout) == 0) << 1;
        stderrCheck = (strcmp(redirectArgs[counter], replaceStderr) == 0) << 2;
        stdCheck = stderrCheck | stdinCheck | stdoutCheck;
        switch(stdCheck)
        {
            if(stdCheck == stdinCheck)
            {
                printf("Stdin Check\n");
                additionalCommands = 0;
                inputRedirectionFile = open(redirectArgs[counter+1], O_RDONLY);
                if(inputRedirectionFile == FAIL_FILE_NOT_FOUND)
                    return;
                dup2(inputRedirectionFile, STDIN_FILENO);
            }
            else if(stdCheck == stdoutCheck)
            {
                printf("Stdout Check\n");
                additionalCommands = 0;
                outputRedirectionFile = open(redirectArgs[counter+1], O_WRONLY | O_CREAT | O_TRUNC, (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH));
                dup2(outputRedirectionFile, STDOUT_FILENO);
            }
            else if(stdCheck == stderrCheck)
            {
                printf("Stderr Check\n");
                additionalCommands = 0;
                outputRedirectionFile = open(redirectArgs[counter+1], O_WRONLY | O_CREAT | O_TRUNC, (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH));          
                dup2(outputRedirectionFile, STDERR_FILENO);
            }
            else
            {
                if(additionalCommands)
                {
                    commands = (char**) realloc(commands, sizeof(char*) * sizeCounter);
                    commands[counter] = redirectArgs[counter];
                }            
            }
        }
        counter++;
    }
    execvp(commands[0], commands);     
}