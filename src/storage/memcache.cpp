#include <iostream>
#include <vector>
#include <map>

using namespace std;

map<int, vector<float> > initMemCache()

{

    map<int, vector<float> > memcache;

    return memcache;
}

//test memcache
/*
int main()
{
    int i;
    map<int, vector<float> > memcache = initMemCache();

    vector<float> v1;
    vector<float> v2;

    for (i = 0; i < 10; i++)
    {
        v1.push_back(i);
        v2.push_back(i + 1);
    }

    memcache[1]= v1;
    memcache.insert(pair<int ,vector<float> >(2, v2));

    vector<float> v3 = memcache[2];
    for (i = 0; i < 10; i++)
    {
        printf("index %d value %f\n", i, v3[i]);
    }
}

*/