#include <iostream> 
#include <ctime> 
#include <cstdlib>
using namespace std;
int main() 
{ 
    srand((unsigned)time(0)); 
    int random_integer; 
    int lowest=1, highest=100; 
    int range=(highest-lowest)+1; 
    for(int index=0; index<20; index++){ 
        random_integer = lowest+int(range*rand()/(RAND_MAX + 1.0)); 
        cout << random_integer << endl; 
    } 
}