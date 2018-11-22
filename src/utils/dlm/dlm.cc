

#include "dlm.h"
#include "unistd.h"

#include <fstream>
#include <string>
#include <thread>
#include <iostream>

#include <atomic>
#include <mutex>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

string lockFileName("fileLock");

//dir is the cluster level
//the path should be sth like ./multinodeip/cluster0

map<string, bool> ifLockMap;

// dir/lockfile

bool atomicCreate(string filePath)
{
    string lockFilePath = filePath + "/" + lockFileName;

    int rvalue = open(filePath.data(), O_CREAT | O_EXCL, 0666);
    //printf("filepath %s return %d\n", filePath.data(), rvalue);
    if (rvalue > 0)
    {
        return true;
    }
    else
    {
        return false;
    }

    
}

//identity could be the addr
void getLock(string Dir, string identity)
{

    string lockFilePath = Dir + "/" + lockFileName;

    while (1)
    {
        bool ifExist = atomicCreate(lockFilePath);

        if (ifExist == false)
        {
            //fail exist
            //check forever
            usleep(1500);
            //printf("identity %s checking\n", identity.data());
        }
        else
        {
            std::ofstream outfile(lockFilePath);

            outfile << identity.data();
            printf("%s acquire lock\n", identity.data());
            outfile.close();
            break;
        }
    }
}

void releaseLock(string Dir)
{
    //if file exist
    //delet file

    string lockFile = Dir + "/" + lockFileName;



        //get content
        //if id match, release

        std::ifstream t(lockFile.data());
        std::string contentStr((std::istreambuf_iterator<char>(t)),
                               std::istreambuf_iterator<char>());

        printf("%s release lock\n", contentStr.data());

        if (remove(lockFile.data()) != 0)
        {
            printf("Error deleting file\n");
        }


    return;
}

void createFile(string FileName)
{
    string Dir = "testDir";

    getLock(Dir.data(), FileName);

    string generateFile = Dir + "/" + FileName;

    std::ofstream outfile(generateFile);

    string info = "test file";

    outfile << info.data() << std::endl;
    printf("%s create file\n", generateFile.data());
    outfile.close();

    sleep(1);

    releaseLock(Dir.data());

    return;
}

/*

int main()
{

    string testDir = "./testDir";

    string command1 = "rm -rf " + testDir;

    system(command1.data());

    string command2 = "mkdir " + testDir;

    system(command2.data());

    //inital value

    int i = 0;
    string File;
    File = "file" + to_string(i) + ".txt";
    printf("file %s\n", File.data());
    thread th0(createFile, File);

    i = 1;
    File = "file" + to_string(i) + ".txt";
    printf("file %s\n", File.data());
    thread th1(createFile, File);

    i = 2;
    File = "file" + to_string(i) + ".txt";
    printf("file %s\n", File.data());
    thread th2(createFile, File);

    i = 3;
    File = "file" + to_string(i) + ".txt";
    printf("file %s\n", File.data());
    thread th3(createFile, File);

    i = 4;

    File = "file" + to_string(i) + ".txt";
    printf("file %s\n", File.data());
    thread th4(createFile, File);

    i = 5;
    File = "file" + to_string(i) + ".txt";
    printf("file %s\n", File.data());
    thread th5(createFile, File);

        i = 6;
    File = "file" + to_string(i) + ".txt";
    printf("file %s\n", File.data());
    thread th6(createFile, File);


        i = 7;
    File = "file" + to_string(i) + ".txt";
    printf("file %s\n", File.data());
    thread th7(createFile, File);

            i = 8;
    File = "file" + to_string(i) + ".txt";
    printf("file %s\n", File.data());
    thread th8(createFile, File);

            i = 9;
    File = "file" + to_string(i) + ".txt";
    printf("file %s\n", File.data());
    thread th9(createFile, File);

            i = 10;
    File = "file" + to_string(i) + ".txt";
    printf("file %s\n", File.data());
    thread th10(createFile, File);
    

    sleep(100);
}
*/