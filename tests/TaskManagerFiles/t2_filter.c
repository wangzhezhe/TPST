#include "stdio.h"

extern "C" int filter(double v, double constraint)
{
        //printf("******Filter func is excuted******\n");
        if (v > constraint)
        {
                printf("data value %f is larger than constraints %f\n", v, constraint);
                return 1;
        }
        else
        {
                return 0;
        }
}