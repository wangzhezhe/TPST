#include "stdio.h"
#include "stdlib.h"
#include <vector>
#include <unistd.h>
#include <omp.h>

using namespace std;

int specificTestFilter(double value, double constraint)
{

    if (value > constraint)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
//TODO optimization, if there is multiple rules for specific Task, the time is waste
int TaskFilterAndAggregation(vector<double> varray)
{
    //get specific TestFilter
    
    printf("start task filter and aggregation operation\n");
    int len = varray.size();

    int i;
    double tempv;
    double redvalue;
    int ifAggre;

    //    omp_set_num_threads(16);

#pragma omp parallel for reduction(+ : redvalue)
    for (i = 0; i < len; i++)
    {
        tempv = varray[i];
        ifAggre = specificTestFilter(tempv, 5);
        if (ifAggre == 1)
        {
            //printf("do filter operation for %f\n", tempv);
            redvalue = redvalue + tempv;
        }
    }

    printf("aggrevalue %f\n", redvalue);

    //if redvalue > constraints
    //trigure the acction

    //printf after reduction operation the value is
    return 0;

    //start new thread to process the filter operation for data array

    //if reture value is true, start the aggregation operation
}

int main()
{

    int vecSize = 200;
    vector<double> varray;
    int count = 0;
    while (1)
    {

        //creat the random number
        //put the number into vector
        int i = 0;
        double temp;
        varray.clear();
        for (i = 0; i < vecSize; i++)
        {
            temp = (double)rand() * 10 / RAND_MAX;
            varray.push_back(temp);
        }
        printf("timestep %d\n", count);
        count++;
        //new time step

        //if filter and aggregation operation finish
        //than call the sleep operation
        TaskFilterAndAggregation(varray);
        usleep(1000000);
    }
    return 0;
}