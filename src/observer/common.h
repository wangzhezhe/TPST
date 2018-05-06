#ifndef common_h
#define common_h

#include <stdlib.h>
#include <stdio.h>
#include <vector>

#define OK EXIT_SUCCESS
#define KO EXIT_FAILURE
#define MAX_OBSERVERS 20

using namespace std;

typedef struct __taskEvent
{
	char str[20];
} TaskEvent;


typedef enum 
{
   BLOCKED = 0,           
   UNBLOCKED
}TaskType;

typedef struct __taskManageMeta
{

	char name[20];
	//aggregation rule
	char aggRule[20];
	//TODO modify this into list
	//interesting to which event
	TaskEvent *subEvent;
	//publish events
	TaskEvent *publishEvent;

	TaskType tasktype;

} TaskManagerMeta;

typedef struct eventHeader
{
	TaskEvent *te;
	//associated task manager meta list
	vector<TaskManagerMeta *> tmList;
} HeadNode;

#endif
