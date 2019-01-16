#include <string>
#include <vector>
#include "./strparse.h"

using namespace std;


vector<int> strSplit(string str)
{
    vector<int> words;
    int leftIdx = 0;
    int rightIdx = 0;
    while (rightIdx < str.size())
    {
        while (rightIdx < str.size() && str[rightIdx] != ',')
            rightIdx++;
        words.push_back(stoi(str.substr(leftIdx, rightIdx - leftIdx)));
        leftIdx = rightIdx + 1;
        rightIdx = rightIdx + 1;
    }

    return words;
}


//GRID[<0,0>:<1,1>]%TS[5] use % to split, then get grid info
//use regx GRID\[.+\]% first
//then get specific data
string getGridFromRawMeta( string publishedMeta){
    
    printf("input str (%s)\n", publishedMeta.data());
    
    int position = publishedMeta.find("%");

    publishedMeta.erase(publishedMeta.begin()+position,publishedMeta.end());
    
    string rawGrid = publishedMeta;
        
    printf("raw grid %s\n",rawGrid.data());

    string GridInner = rawGrid.substr(5,rawGrid.length()-6);
    
    return GridInner;
}

//use regx TS\[.+\] first
int getTsFromRawMeta( string publishedMeta){
    int position = publishedMeta.find("%");

    publishedMeta.erase(publishedMeta.begin(),publishedMeta.begin()+position+1);

    string rawTS = publishedMeta;

    printf("raw ts %s\n",rawTS.data());

    int TS = stoi(rawTS.substr(3,rawTS.length()-4));

    return TS;
}


//the meta string should be this format <0,0>:<2,2>
void getbbxfromGridMeta(string gridRawMeta, vector<int> &lb, vector<int> &ub)
{

    printf("get gridRawMeta %s\n",gridRawMeta.data());
    
    int startPosition = gridRawMeta.find(":");
    string lbmeta = gridRawMeta;
    string ubmeta = gridRawMeta;

    lbmeta.erase(lbmeta.begin() + startPosition, lbmeta.end());
    ubmeta.erase(0, startPosition + 1);

    lbmeta = lbmeta.substr(0, lbmeta.length() - 1);
    lbmeta = lbmeta.substr(1, lbmeta.length());

    ubmeta = ubmeta.substr(0, ubmeta.length() - 1);
    ubmeta = ubmeta.substr(1, ubmeta.length());

    lb = strSplit(lbmeta);
    ub = strSplit(ubmeta);

    return;
}