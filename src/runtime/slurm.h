

#ifndef slurm_h
#define slurm_h

//TODO make this a common defination 
typedef int (*runtimeFunc)(char *);

int slurmTaskStart(const char *batchPath);
#endif