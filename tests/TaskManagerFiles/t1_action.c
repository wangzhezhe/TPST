#include "stdio.h"

extern "C" 
{
int action(double v, double constraint)
{
    printf("******Action func is excuted******\n");
    if (v > constraint)
    {
        printf("data value is larger than constraints");
        return 1;
    }
    else
    {
        return 0;
    }
}
}