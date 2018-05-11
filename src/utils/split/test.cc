

#include "split.h"
#include "stdlib.h"
#include "stdio.h"
#include <string>

int main()
{
    string s = "event1:1:2";
    vector<string> v = split(s, ":"); //this could be splited by multiple split characters
    for (vector<string>::size_type i = 0; i != v.size(); ++i)
    {
        cout << v[i] << " "<<endl;
    }

    if (v.size()==2){
        //stoi is suppotted by -std=c++11
        int a= stoi(v[1]);
        printf("num %d\n",a);
    }
}