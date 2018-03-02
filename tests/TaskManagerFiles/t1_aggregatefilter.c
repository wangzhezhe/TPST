#include "stdio.h"

extern "C" {
int aggregatefilter(double reductValue,double constraints)
{
        if(reductValue>constraints){
                printf("aggregate value larger than constraints value publish event\n");
                return 1;
        }else{
                return 0;
        }

}
}