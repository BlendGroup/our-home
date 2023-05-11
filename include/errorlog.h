#ifndef __ERROR_LOG__
#define __ERROR_LOG__
#include <string>
#ifdef DEBUG
#define throwErr(msg) throw ("[" + std::string(__FILE__) + " " + std::to_string(__LINE__) + "] error :" + std::string(msg))
#else
#define throwErr(msg)
#endif

#endif