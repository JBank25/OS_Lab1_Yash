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


char ** parseInput(char * input);

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