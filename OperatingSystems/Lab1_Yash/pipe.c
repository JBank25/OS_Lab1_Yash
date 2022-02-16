/**
 * @file pipe.c
 * @author your name (you@domain.com)
 * @brief Pipes are unidirectional communication lines between two process 
 * which have parent-child relationships.
 * @version 0.1
 * @date 2022-02-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "jobs.h"
#include "file_redirection.h"

#define READ_END 0
#define WRITE_END 1

/**
 * @brief
 *  | separates 2 commands, left command has stdout replaced
 * with the input to a pipe. Right command has stdin replaced
 * with the output from the same pipe. Children with the same
 * pipeline will be in one process group for the pipelien.
 * Children with the same pipeline will be started and stopped
 * simulatneously. 
 * 
 * @param leftCmd 
 * @param rightCmd
 */
void handlePipe(process_t* process1, process_t* process2, int jpid)
{
    int pipefd[2];
    pid_t cpid;
    int cpidSave, status1, status2;
    if(pipe(pipefd) == -1)
    {
        printf("ERROR WITH PIPE!\n");
        return;
    }
    cpid = fork();
    if(cpid == -1)
    {
        printf("ERROR FORKING\n");
    }
    if(cpid == 0)//child process
    {
        close(pipefd[READ_END]);//closing read end of the pipe
        dup2(pipefd[WRITE_END], STDOUT_FILENO);
        close(pipefd[WRITE_END]);
        handleProcess(process1);
    }
    else
    {
        cpidSave = cpid;
        cpid = fork();
        if(cpid == 0)
        {
            setpgid(0,jpid);//second child now also part of the same pgid
            dup2(pipefd[READ_END], STDIN_FILENO);
            close(pipefd[WRITE_END]);
            close(pipefd[READ_END]);//closing read end of the pipe
            handleProcess(process2);  
        }
        else
        {
            close(pipefd[READ_END]);
            close(pipefd[WRITE_END]);
            waitpid(cpidSave,&status1,WUNTRACED);
            waitpid(cpid, &status2, WUNTRACED);
        }
    }
}


/**
 * @brief Get the index in char** inputs of the pipe if it exists
 * 
 * @param inputs 
 * @return int index of pipe in the char** input
 */
int getPipeIndex(char ** inputs)
{
    int i = 0;
    while(inputs[i] != NULL)
    {
        if(strcmp(inputs[i], "|\0") == 0)
            return i;
        i++;
    }
    return -1;
}