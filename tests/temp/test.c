#include <stdio.h>
#include <pthread.h>

// remember to set compilation option-pthread
// refer to https://stackoverflow.com/questions/2251452/how-to-return-a-value-from-thread-in-c

void *busy(void *ptr)
{
    printf("***********threadIndex %d is doing sth\n", *((int *)ptr));
    
    sleep(2);
    
    return (void*)12345;
}

int main()
{

    pthread_t id;
    pthread_t self;
    int status;
    int threadNum = 10;

    int i = 0;
    int tIndex = 0;
    
    self = pthread_self();
    printf("current thread id %d\n", self);

    pthread_create(&id, NULL, busy, (void *)&tIndex);
    printf("waiting the terminal of threads id %d\n", id);
    pthread_join(id, (void**)&status);
    //print the return value of the thread
    printf("return value (%d) of the pthread (%d)\n", (int)status, id);
   
    printf("main thread finish, all thread is finished\n");
}