#include "stdio.h"
#include <unistd.h>
#include "../../observer/taskcommon.h"
#include <vector>
#include <map>
#include <string>

using namespace std;
// reference to db
// reference to es
extern "C" int action(vector<HeadNode *> &es, map<string, vector<float> > storage)
{
    int size = es.size();
    printf("size of ex %d\n", size);

    //create data and put the data into the storage

    int vecSize = 1000;
    int count = 0;
    string key = "task1";

    while (1)
    {
        map<string, vector<float> >::iterator iter;
        // fingding the key in map
        iter = storage.find(key);
        vector<float> varray;
        if (iter != storage.end())
        {
            // second variable is value
            varray = iter->second;
        }
        else
        {
            printf("init an new array\n");
            storage[key] = varray;
        }

        //creat the random number
        //put the number into vector
        int i = 0;
        double temp;
        srand(time(0));
        for (i = 0; i < vecSize; i++)
        {
            temp = (double)rand() * 10 / RAND_MAX;
            varray.push_back(temp);
        }

        //printf("current value in storage\n");
        //for (i = 0; i < vecSize; i++)
        //{
        //    printf("index %d value %lf\n", i, varray[i]);
        //}
        //count++;
        //new time step

        //if filter and aggregation operation finish
        //than call the sleep operation
        //vector<int> *v = static_cast<vector<int> *>(args);

        //simulate every steps
        usleep(2000000);
    }
}
