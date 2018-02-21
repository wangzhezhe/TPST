
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

int ifjson(char *filename)
{
    if (strstr(filename, "swp") != NULL)
    {

        return 0;
    }

    if (strstr(filename, "json") != NULL)
    {
        //neglect json.swp
        return 1;
    }
    else
    {
        return 0;
    }
}