#include "../lib/rapidjson/include/rapidjson/document.h"
#include "../lib/rapidjson/include/rapidjson/writer.h"
#include "../lib/rapidjson/include/rapidjson/stringbuffer.h"
#include "../lib/file/loaddata.h"
#include <iostream>
#include "stdlib.h"
#include "stdio.h"

using namespace rapidjson;

int main()
{
    // 1. Parse a JSON string into DOM.
    // load file
    char *filename = "./TaskManagerFiles/tm1.json";
    char *jsonbuffer = NULL;
    jsonbuffer = loadFile(filename);

    printf("jsonbuffer %s\n", jsonbuffer);

    Document d;
    d.Parse(jsonbuffer);

    // 2. Modify it by DOM.
    Value &s = d["name"];
    //printf name

    printf("task name %s\n", d["name"].GetString());

    // 3. Stringify the DOM
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);

    // Output {"project":"rapidjson","stars":11}
    std::cout << buffer.GetString() << std::endl;

    return 0;
}