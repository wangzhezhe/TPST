#include "time.h"
#include "stdlib.h"
#include "stdio.h"
#include "local.h"
#include "../utils/strparse/strparse.h"
#include <string>
#include <mutex>
#include <Python.h>
#include <thread>

using namespace std;

mutex testNotifiedNumMtx;
int localFinishNum = 0;

double call_func(PyObject *func, double x, double y)
{
    PyObject *args;
    PyObject *kwargs;
    PyObject *result = 0;
    double retval;

    /* Make sure we own the GIL */
    PyGILState_STATE state = PyGILState_Ensure();

    /* Verify that func is a proper callable */
    if (!PyCallable_Check(func))
    {
        fprintf(stderr, "call_func: expected a callable\n");
        goto fail;
    }
    /* Build arguments */
    args = Py_BuildValue("(dd)", x, y);
    kwargs = NULL;

    /* Call the function */
    result = PyObject_Call(func, args, kwargs);
    Py_DECREF(args);
    Py_XDECREF(kwargs);

    /* Check for Python exceptions (if any) */
    if (PyErr_Occurred())
    {
        PyErr_Print();
        goto fail;
    }

    /* Verify the result is a float object */
    if (!PyFloat_Check(result))
    {
        fprintf(stderr, "call_func: callable didn't return a float\n");
        goto fail;
    }

    /* Create the return value */
    retval = PyFloat_AsDouble(result);
    Py_DECREF(result);

    /* Restore previous GIL state and return */
    PyGILState_Release(state);
    return retval;

fail:
    Py_XDECREF(result);
    PyGILState_Release(state);
    abort(); // Change to something more appropriate
}

/* Load a symbol from a module */
PyObject *import_name(const char *modname)
{
    PyObject *u_name, *module = NULL;

    printf("debug1\n");
    u_name = PyUnicode_FromString(modname);
    printf("debug2\n");
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('/project1/parashar-001/zw241/software/eventDrivenWorkflow/tests/movingGridMultiInStagingEvent/')");
    //PyRun_SimpleString("sys.path.append('/project1/parashar-001/zw241/software/eventDrivenWorkflow/src/publishclient/pythonclient/')");
    //PyRun_SimpleString("sys.path.append('./')");

    module = PyImport_Import(u_name);
    printf("debug3\n");
    if (module == NULL)
    {
        printf("import module faile\n");
        exit(0);
    }
    printf("debug4\n");

    PyObject *test = NULL;
    test = PyObject_GetAttrString(module, "checkIteration");
    //test = PyObject_GetAttrString(module, "checkIteration");
    if (test == NULL)
    {
        printf("import failed\n");
        exit(0);
    }

    return test;
}

PyObject *py_func = NULL;

void callPythonCode(int ts)
{
    if (py_func == NULL)
    {
        Py_Initialize();
        /* Get a reference to the math.pow function */
        //pow_func = import_name("math","pow");
        printf("before import\n");
        py_func = import_name("analytics");
        //py_func = import_name("testPythonFunc");
        /* Call it using our call_func() code */
        printf("after import\n");
    }

     PyObject *args;
    PyObject *kwargs;

    args = Py_BuildValue("(i)", ts);

    kwargs = NULL;

    /* Call the function */
    PyObject *result = PyObject_Call(py_func, args, kwargs);

    if (!result)
    {
        printf("invoke function fail\n");
    }

    /* Done */
    //Py_DECREF(py_func);
    //Py_Finalize();
}

int pythonTaskStart(string batchPath, string metadata)
{
    //get ts
    printf("debug start python task execution\n");
    int ts = getTsFromRawMeta(metadata);

    //call sbtch command
    //printf("test output\n");
    char command[200];
    //in case that the number of job is larger than limitation
    //#ifdef TIME
    //send publish api and record time

    //printf("finish sec:(%ld),finish nsec:(%ld)\n",finish.tv_sec,finish.tv_nsec);

    //#endif
    sprintf(command, "%s %s %s", "python", batchPath.data(), to_string(ts).data());

    printf("execute command by python way:(%s)\n", command);

    //test using
    system(command);
}

int localTaskStart(string batchPath, string metadata)
{

    //get ts
    int ts = getTsFromRawMeta(metadata);

    //call sbtch command
    //printf("test output\n");
    //there is limitaiton for sbatch number
    if (ts > 0 && (ts + 1) % 10 == 0)
    {
        char command[200];
        //in case that the number of job is larger than limitation
        //#ifdef TIME
        //send publish api and record time

        //printf("finish sec:(%ld),finish nsec:(%ld)\n",finish.tv_sec,finish.tv_nsec);

        //#endif
        sprintf(command, "%s %s", batchPath.data(), to_string(ts).data());

        printf("execute command by local way:(%s)\n", command);

        //test using

        system(command);

        //c call python

        //thread pubthread(callPythonCode, ts);
        //pubthread.detach();
    }

    //struct timespec finish;
    //auto console = spd::stdout_color_mt("console");
    //spdconsole->info("finish sec:({:d}),finish nsec:({:d})", finish.tv_sec, finish.tv_nsec);

    //testNotifiedNumMtx.lock();
    //localFinishNum++;
    //testNotifiedNumMtx.unlock();

    return 0;
}