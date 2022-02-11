#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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
void yashPipe(char* leftCmd, char* rightCmd)
{

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
        if(strcmp(inputs[i], "|") == 0)
            return i;
        i++;
    }
    return 0;
}