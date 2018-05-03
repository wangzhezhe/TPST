

#ifndef log_h
#define log_h
#include "spdlog/spdlog.h"

//refer to https://stackoverflow.com/questions/2029272/how-to-declare-a-global-variable-that-could-be-used-in-the-entire-program
//this could also be used to implement singelton mode

namespace spd = spdlog;
auto spdconsole = spd::stdout_color_mt("console");

#endif