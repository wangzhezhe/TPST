#ifndef taskcommon_h
#define taskcommon_h

#include "common.h"

#include <vector>
#include <map>
#include <string>


using namespace std;



typedef int (*actionFunc)(vector<HeadNode *>, map<string, vector <float> >);

typedef int (*filterFunc)(double, double);

typedef int (*aggFilterFunc)(double, double);

/**
	 * Defining a 'interface-a-like' for the observer
**/

typedef struct __observer
{
	/**
		 * Method for performing garbage collection
		 */
	//void (*destroy)(struct __observer *);
	/**
		 * A reference to the taskmanager
		 */
	//if use TaskManager* , it will cause circulation reference
	void *tm;
	/**
		 * A method that any subject is able to trigger
		 */
	//void (*notifyfunc)(Observer*, int, void *);
	actionFunc actionfunc;

	filterFunc filterfunc;

	aggFilterFunc aggfilterfunc;
	/**
		 * The notify implementation (the actual function that is going to
		 * be triggered)
		 */
	//void (*notifyImpl)(void*, int, void*);
} Observer;

typedef struct __subject
{
	int type;
	/**
		 * Destructor for the Subject
		 */
	void (*destroy)(struct __subject *);
	/**
		 * Referente to the concrete Subject object
		 */
	void *impl;
	/**
		 * A list of observes have ben are registered for this subject
		 * object.
		 */
	Observer *observers[MAX_OBSERVERS];
	/**
		 * The method that allows observers to register for this
		 * subject object.
		 */
	int (*registerObserver)(struct __subject *, Observer *);
	/**
		 * Lets observes unregister from this subject
		 */
	int (*unregisterObserver)(struct __subject *, Observer *);
	/**
		 * When fired, triggers notify method of all of the registered
		 * observers.
		 */
	void (*notifyObservers)(struct __subject *);
} Subject;


typedef struct __taskManager
{
	TaskManagerMeta * taskmeta;
	Subject *subject;
	Observer *observer;

} TaskManager;



#endif
