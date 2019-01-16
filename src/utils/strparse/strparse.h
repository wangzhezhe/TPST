#ifndef STRPARSE_H
#define STRPARSE_H

#include <vector>
#include <string>

using namespace std;

void getbbxfromGridMeta(string gridRawMeta, vector<int> &lb, vector<int> &ub);
int getTsFromRawMeta(const string publishedMeta);
string getGridFromRawMeta( string publishedMeta);

#endif