

#include "stdio.h"
#include "stdlib.h"

#include "slurm.h"

int main(){
    int r=slurmTaskStart("testfile");
    printf("return value %d\n",r);
    if (r>0){
        printf("command execute fail\n");
    }
    
}