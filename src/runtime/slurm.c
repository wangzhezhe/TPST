#include "stdlib.h"
#include "stdio.h"


 int slurmTaskStart(const char *batchPath){
     //call sbtch command
     char command [200];
     sprintf(command,"sbatch %s",batchPath);
     printf("execute command:(%s)\n",command);
     return system(command);
 }