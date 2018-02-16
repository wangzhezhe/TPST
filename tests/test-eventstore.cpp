#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "../eventstore/eventStore.h"

// g++ -o test-event ./test-eventstore.cpp ../eventstore/eventStore.cpp ../observer/taskmanager.c ../observer/observer.c

static void notify_F1()
{
    printf("call f1\n");
    Task_handleEvent();
}
static void notify_F2()
{
    printf("call f2\n");
    Task_handleEvent();
}

int main()
{
    vector<HeadNode *> es = initEventStore();
    int size = es.size();
    //printf("size of cache %d\n", size);

    //test register event
    //int registerEvent(vector<HeadNode *> es, TaskEvent *te, TaskManager *tm)

    char t1finish[20] = "T1_FINISH";
    char t2finish[20] = "T2_FINISH";
    TaskEvent *te1 = (TaskEvent *)malloc(sizeof(TaskEvent));
    memcpy(te1->str, t1finish, strlen(t1finish));

    TaskEvent *te2 = (TaskEvent *)malloc(sizeof(TaskEvent));
    memcpy(te2->str, t2finish, strlen(t2finish));

    TaskManager *t1 = Task_create("t1");

    TaskManager *t2 = Task_create("t2");

    TaskManager *t3 = Task_create("t3");

    registerEvent(es, te1, t1);

    registerEvent(es, te1, t2);

    registerEvent(es, te2, t3);

    printEventStore(es);

    //register the function
    Task_registerAction(t1, notify_F1);

    Task_registerAction(t2, notify_F2);

    //test notify specific event
    
    notifyTmList(es, te1);

    return 0;
}