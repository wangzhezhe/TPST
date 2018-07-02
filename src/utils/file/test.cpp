

#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include "loaddata.h"

using namespace std;

int main()
{
    /*
    const char *dir = "/home/parallels/Documents/cworkspace/observerchain/tests/TrigureFiles";
    vector<string> strList;
    strList=scanFolder(dir);

    printf("print return list\n");
    
    int count = strList.size();
    
    for (int i = 0; i < count;i++)

    {
        cout << strList[i] << endl;
    }
    */
   char *file="./broadcaster/trigure_3ana.json";
   string filecontent;
   filecontent=loadFile(file);
   printf("load data (%s)\n",filecontent.data());
}