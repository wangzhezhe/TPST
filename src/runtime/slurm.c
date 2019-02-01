#include "time.h"
#include "stdlib.h"
#include "stdio.h"
#include "slurm.h"
//#include "../../src/utils/log/log.h"

//namespace spd = spdlog;

 int slurmTaskStart(const char *batchPath){
     //call sbtch command
     //printf("test output\n");
     char command [200];
//in case that the number of job is larger than limitation
//#ifdef TIME
    //send publish api and record time
    struct timespec finish;
    clock_gettime(CLOCK_MONOTONIC, &finish);
    //auto console = spd::stdout_color_mt("console");
    //spdconsole->info("finish sec:({:d}),finish nsec:({:d})",finish.tv_sec,finish.tv_nsec);
    //printf("finish sec:(%ld),finish nsec:(%ld)\n",finish.tv_sec,finish.tv_nsec);

//#endif
     sprintf(command,"sbatch %s",batchPath);
#ifdef DEBUG
     printf("execute command:(%s)\n",command);
#endif
     
     //test using
     return 0;
     //return system(command);
 }