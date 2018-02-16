
#include "taskcommon.h"




TaskManager *Task_create(char *tmname);

int Task_destroy(TaskManager *tm);

int Task_registerAction(TaskManager *tm, notifyFunc notifyfun);

void Task_handleEvent();

int Task_pushevent(TaskManager *tm, char *event_str);
int Task_listen(vector<HeadNode *> &es, TaskManager *tm, char *event_str);

int registerEvent(TaskManager *tm,TaskEvent *te, vector<HeadNode *> &es);

int notifyTmList(TaskManager *tm, vector<HeadNode *> &es, TaskEvent *te);


int callNotify(TaskManager *tm,vector<HeadNode *> &es,TaskEvent*te, Observer *obs);