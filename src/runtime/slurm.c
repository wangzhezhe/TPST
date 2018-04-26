#include "time.h"
#include "stdlib.h"
#include "stdio.h"


 int slurmTaskStart(const char *batchPath){
     //call sbtch command
     char command [200];
     sprintf(command,"sbatch %s",batchPath);
#ifdef DEBUG
     printf("execute command:(%s)\n",command);
#endif

#ifdef TIME
    //send publish api and record time
    struct timespec finish;
    clock_gettime(CLOCK_MONOTONIC, &finish);
    printf("finish sec:(%ld),finish nsec:(%ld)\n",finish.tv_sec,finish.tv_nsec);

#endif
     return system(command);
 }