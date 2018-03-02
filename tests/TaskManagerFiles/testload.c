#include "stdlib.h"
#include "stdio.h"
#include "string.h"


int main(){
    char *filename="/home1/zw241/observerchain/tests/TaskManagerFiles/tm0.json";
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
        
    }
    
    return 0;
}