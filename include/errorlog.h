#ifndef __ERROR_LOG__
#define __ERROR_LOG__

#ifdef DEBUG
#define throwErr(msg) throw ("[" + string(__FILE__) + " " + to_string(__LINE__) + "] error :" + string(msg))
#else
#define throwErr(msg)
#endif

#endif