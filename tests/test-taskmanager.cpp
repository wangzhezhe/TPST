/**
 * Functional testing for the Observer
 */

#include <stdio.h>
#include "../observer/taskmanager.h"

static void notify_F1()
{
	printf("call f1\n");
	Task_handleEvent();
	//get event message from tm for finishing execution

	//send messge to event store ,call notify
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

int main()
{
	char t1finish[20] = "T1_FINISH";
	char t2finish[20] = "T2_FINISH";
	char t3finish[20] = "T3_FINISH";

	//create task
	TaskManager *t1 = Task_create("t1");

	TaskManager *t2 = Task_create("t2");

	TaskManager *t3 = Task_create("t3");
	
	//subscribe or pushlish event
	//In real case, there is filter function should be used to control the event registration
	Task_pushevent(t1, t1finish);

	//Task_subscribe(t2, t1finish);

	Task_pushevent(t2, t2finish);

	//Task_subscribe(t3, t2finish);

	Task_pushevent(t3, t3finish);

	//register action function
	//assume there is one subject and one observer for every task
	//register the function into the observer of every task
	Task_registerAction(t1, notify_F1);

	Task_registerAction(t2, notify_F2);

	Task_registerAction(t3, notify_F3);

	//start t1 manually

	//Task_notify(t1);

	//Task_destroy(tm);


	//test call notify
	//callNotify(t1->observer);
	
	printf("tm size %d %s\n", sizeof(TaskManager), "test ok");
	
	return 0;
}
