
#include "taskcommon.h"

TaskManager *Task_create(const char *tmname, const char *aggRule, vector<HeadNode *> &es);
TaskManager *getTmfromES(const char *tmname, vector<HeadNode *> &es);
int Task_destroy(TaskManager *tm);

int Task_registerAction(TaskManager *tm, notifyFunc notifyfun);

int Task_registerFilter(TaskManager *tm, filterFunc filterfunc);

int Task_registerAggreFilter(TaskManager *tm, aggFilterFunc aggfilterfunc);

int deleteTmfromES(TaskManager *tm, vector<HeadNode *> &es);

void Task_handleEvent();

int Task_pushevent(TaskManager *tm, const char *event_str);

int Task_subscribe(vector<HeadNode *> &es, TaskManager *tm, const char *event_str);

int registerEvent(TaskManager *tm,TaskEvent *te, vector<HeadNode *> &es);

int notifyTmList(TaskManager *tm, vector<HeadNode *> &es, TaskEvent *te);

int callNotify(TaskManager *tm,vector<HeadNode *> &es,TaskEvent*te, Observer *obs);

int TaskFilterAndAggregation( vector<double> dataArray, vector<HeadNode *> &es);


int TaskFilter(TaskManager * tm, double value, double constraint);

int TaskReductionFilter(TaskManager * tm, double value, double constraint);

int IfFilterAggregation(TaskManager *tm);