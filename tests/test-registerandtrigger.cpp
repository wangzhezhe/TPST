#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "../observer/taskmanager.h"
#include "../eventstore/eventStore.h"

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
static void notify_F3()
{
    printf("call f3\n");
    Task_handleEvent();
}

static void notify_F4()
{
    printf("call f4\n");
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
    char t3finish[20] = "T3_FINISH";
    char t4finish[20] = "T4_FINISH";

    //create task
    TaskManager *t1 = Task_create("t1");

    TaskManager *t2 = Task_create("t2");

    TaskManager *t3 = Task_create("t3");

    TaskManager *t4 = Task_create("t4");

    //subscribe or publish what event
    //In real case, there is filter function should be used to control the event registration

    //t1 will send the t1finish event to the event store after function finish
    Task_pushevent(t1, t1finish);

    Task_subscribe(es, t2, t1finish);

    Task_pushevent(t2, t2finish);

    Task_subscribe(es, t3, t1finish);

    Task_pushevent(t3, t3finish);

    Task_subscribe(es, t4, t2finish);

    Task_pushevent(t4, t4finish);

    //register action function
    //assume there is one subject and one observer for every task
    //register the function into the observer of every task
    Task_registerAction(t1, notify_F1);

    Task_registerAction(t2, notify_F2);

    Task_registerAction(t3, notify_F3);

    Task_registerAction(t4, notify_F4);

    printEventStore(es);

    printf("trigure t1 mannualy");

    callNotify(t1, es, t1->publishEvent, t1->observer);

    printf("delete t3\n");
    //test delete function
    //TODO fix bug when deleting t1
    deleteTmfromES(t2, es);
    printEventStore(es);

    return 0;
}