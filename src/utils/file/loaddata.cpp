
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <fstream>
#include <exception>
#include <iostream>
using namespace std;

/*
char *loadFile(char *filename)
{
    FILE *f = fopen(filename, "rb");
    char *buffer = NULL;
    if (f)
    {
        fseek(f, 0, SEEK_END);
        int length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = (char *)malloc(length);
        if (buffer)
        {
            fread(buffer, 1, length, f);
        }
        fclose(f);
    }
    else
    {
        printf("failed to load file %s\n", filename);
        return NULL;
    }

    return buffer;
}
*/

string loadFile(const char *filename)
{

    ifstream ifs;
    ifs.open(filename);
    if (ifs.fail())
    {
        printf("ifs fail\n");
        exit(1);
    }

    string content((std::istreambuf_iterator<char>(ifs)),
                   (std::istreambuf_iterator<char>()));

    return content;
}

char *getTaskNameFromEventName(char *filename)
{
    if (strstr(filename, "swp") != NULL)
    {

        return NULL;
    }

    if (strstr(filename, "swx") != NULL)
    {

        return NULL;
    }
    if (strstr(filename, "json") != NULL)
    {
        //check last four character

        int len = strlen(filename);
        char *tempstr = (char *)malloc(sizeof(filename));

        //.json should be deleted
        strncpy(tempstr, filename, len - 5);
        tempstr[len - 5] = '\0';
        printf("extract file name %s\n", tempstr);
        return tempstr;
    }
    else
    {
        return NULL;
    }
}

int ifjson(char *filename)
{
    if (strstr(filename, "swp") != NULL)
    {

        return 0;
    }

    if (strstr(filename, "swx") != NULL)
    {

        return 0;
    }

    if (strstr(filename, "json") != NULL)
    {
        //check last four character
        int len = strlen(filename);
        char tempstr[10];
        strncpy(tempstr, filename + len - 4, 5);
        //printf("debug test tempstr %s\n", tempstr);
        if (strcmp(tempstr, "json") == 0)
        {
            return 1;
        }
        else
        {
            return 0;
        }

        return 1;
    }
    else
    {
        return 0;
    }
}

//scan folder (one level) and load the .json files
vector<std::string> scanFolder(const char *dirpath)
{

    DIR *dp;
    vector<string> fileList;
    struct dirent *entry;
    struct stat statbuf;

    if ((dp = opendir(dirpath)) == NULL)
    {
        fprintf(stderr, "Can`t open directory %s\n", dirpath);
        return fileList;
    }

    while ((entry = readdir(dp)) != NULL)
    {

        string temps = entry->d_name;
        fileList.push_back(temps);
    }

    //printf("debug 3\n");

    return fileList;
}