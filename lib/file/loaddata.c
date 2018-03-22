
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

char *loadFile(char *filename)
{
    FILE *f = fopen(filename, "rb");
    char *buffer = NULL;
    if (f)
    {
        fseek(f, 0, SEEK_END);
        int length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = (char *)malloc(length);
        if (buffer)
        {
            fread(buffer, 1, length, f);
        }
        fclose(f);
    }
    else
    {
        printf("failed to load file %s\n", filename);
        return NULL;
    }

    return buffer;
}

char *getTaskNameFromEventName(char *filename)
{
    if (strstr(filename, "swp") != NULL)
    {

        return NULL;
    }

    if (strstr(filename, "swx") != NULL)
    {

        return NULL;
    }
    if (strstr(filename, "json") != NULL)
    {
        //check last four character

        int len = strlen(filename);
        char *tempstr = (char *)malloc(sizeof(filename));
        
        //.json should be deleted
        strncpy(tempstr, filename, len - 5);
        tempstr[len - 5] = '\0';
        printf("extract file name %s\n", tempstr);
        return tempstr;
    }
    else
    {
        return NULL;
    }
}

int ifjson(char *filename)
{
    if (strstr(filename, "swp") != NULL)
    {

        return 0;
    }

    if (strstr(filename, "swx") != NULL)
    {

        return 0;
    }

    if (strstr(filename, "json") != NULL)
    {
        //check last four character
        int len = strlen(filename);
        char tempstr[10];
        strncpy(tempstr, filename + len - 4, 5);
        //printf("debug test tempstr %s\n", tempstr);
        if (strcmp(tempstr, "json") == 0)
        {
            return 1;
        }
        else
        {
            return 0;
        }

        return 1;
    }
    else
    {
        return 0;
    }
}