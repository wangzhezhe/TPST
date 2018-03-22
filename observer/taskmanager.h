
#ifndef taskmanager_h
#define taskmanager_h

#include "taskcommon.h"

#include "../lib/rapidjson/include/rapidjson/document.h"
#include "../lib/rapidjson/include/rapidjson/writer.h"
#include "../lib/rapidjson/include/rapidjson/stringbuffer.h"

using namespace rapidjson;

extern map<string, TaskManager *> taskManagerGlobal;

//use es (event store) as a global variable
extern vector<HeadNode *> es;

//this should be defined in the taskmanager.cpp
extern map<string, vector<float> > storage;

TaskManager *Task_create(const char *tmname, const char *aggRule, const char *subevent, const char *pubevent, vector<HeadNode *> &es, TaskType taskType);

TaskManager *getTmfromES(const char *tmname, vector<HeadNode *> &es);

int Task_destroy(TaskManager *tm);

int Task_registerAction(TaskManager *tm, actionFunc actionfun);

int Task_registerFilter(TaskManager *tm, filterFunc filterfunc);

int Task_registerAggreFilter(TaskManager *tm, aggFilterFunc aggfilterfunc);

int deleteTmfromES(TaskManager *tm, vector<HeadNode *> &es);

void Task_handleEvent();

int Task_pushevent(TaskManager *tm, const char *event_str);

int Task_subscribe(vector<HeadNode *> &es, TaskManager *tm, const char *event_str);

int registerEvent(TaskManagerMeta *tmmeta, TaskEvent *te, vector<HeadNode *> &es);

int notifyTmList(TaskManager *tm, vector<HeadNode *> &es, TaskEvent *te);

int callNotify(TaskManager *tm,vector<HeadNode *> &es,TaskEvent*te, Observer *obs);

int TaskFilterAndAggregation( vector<double> dataArray, vector<HeadNode *> &es);


int TaskFilter(TaskManager * tm, double value, double constraint);

int TaskReductionFilter(TaskManager * tm, double value, double constraint);

int IfFilterAggregation(TaskManager *tm);

void TaskFileParsing(Document &d, const char *taskName, char *jsonbuffer,char*taskPath,char*projectPath,char*tmDir);


#endif