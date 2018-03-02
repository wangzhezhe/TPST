#include <iostream>
#include <vector>
#include <pthread.h>

using namespace std;

void *func(void *args)
{
    vector<int> *v = static_cast<vector<int> *>(args);
    cout << "Vector size in new thread: " << v->size()<<endl;
}

int main(int argc, char *argv[])
{

    vector<int> integers;
    pthread_t thread;

    for (int i = 0; i < 10; i++)
        integers.push_back(i + 1);

    // overheat call
    //pthread_create( &thread, NULL, func, static_cast<void*>(&integers));

    pthread_create(&thread, NULL, func, &integers);
    pthread_join(thread, NULL);
    cout << "Main thread finalized" << endl;

    return 0;
}