#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @Checks input for any invalid first command
 * 
 * @param input 
 * @return -1 if invalid 1 if valid
 */
int cmdCheck(char* input)
{
    int invalidCount = 5;
    char** invalidInputs = {"<", ">", "2>", "|", "&"};
    for(int i = 0; i < invalidCount; i++)
    {
        if(strcmp(invalidInputs[i], input) == 0)
            return -1;
    }
    return 1;
}