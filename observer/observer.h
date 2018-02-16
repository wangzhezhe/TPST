#ifndef observer_h
#define observer_h

#include <vector>
#include "./taskcommon.h"

using namespace std;

/**
	 * Constructor, allocate memory and set up credentials.
	 * @param vendor name of the observer manufacturer
	 * @param serial serial number
	 * @return an instance of Observer
	 */
Observer *observerNew(void *tm, notifyFunc notifyfunc);

int callNotify(vector<HeadNode *> &es, TaskEvent *te, Observer *obs);

#endif
