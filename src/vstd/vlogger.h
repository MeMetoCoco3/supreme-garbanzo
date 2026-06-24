#ifndef LOGGER
#define LOGGER

#include "vtypes.h"
#include <stdarg.h>
#include <cstdio>
// ## Deletes preciding coma, so the function will still work without arguments
#define V_LOG_INFO(msg, ...) Logger::Get()->Info(call_location{__FILE__, __func__, __LINE__}, msg, ##__VA_ARGS__)  
#define V_LOG_WARN(msg, ...) Logger::Get()->Warn(call_location{__FILE__, __func__, __LINE__}, msg, ##__VA_ARGS__)  
#define V_LOG_ERROR(msg, ...) Logger::Get()->Error(call_location{__FILE__, __func__, __LINE__}, msg, ##__VA_ARGS__) 

typedef struct call_location{
    const char * file;
    const char * func;
    int loc;
} call_location;


class Logger 
{
    public: 
	enum Level: u8{
	    L_INFO = 1<<0, 
	    L_WARN = 1<<1, 
	    L_ERROR = 1<<2 
	};

    public:
	inline static u8 m_LogLevel = L_INFO | L_WARN | L_ERROR;
    static Logger* Get() {
        static Logger* logger = new Logger();
        return logger;
    }

	void SetLevelDefault(void) {
		m_LogLevel = (L_INFO | L_WARN | L_ERROR);
	}

	void SetLevel(u8 level)
	{
	    m_LogLevel = level;
	}
	
	void Warn(call_location c_loc, const char *msg ...)
	{
	    if ((m_LogLevel & L_WARN) == L_WARN){
            printf("\033[38;5;226m[WARNING]:");
            va_list args;
            va_start(args, msg);
            vprintf(msg, args);
            va_end(args);
            printf("%s %s Line:%i\n", c_loc.file, c_loc.func, c_loc.loc);
            printf("\033[0m");
        }
	}

	void Error(call_location c_loc, const char *msg, ...)
	{
	    if ((m_LogLevel & L_ERROR) == L_ERROR){
            printf("\033[38;5;160m[ERROR]:");
            va_list args;
            va_start(args, msg);
            vprintf(msg, args);
            va_end(args);
            printf("%s %s Line:%i\n", c_loc.file, c_loc.func, c_loc.loc);
            printf("\033[0m");
        }
	}

	void Info(call_location c_loc, const char *msg, ...)
	{
	    if ((m_LogLevel & L_INFO) == L_INFO){
            printf("\033[0;32m[INFO]:");
            va_list args;
            va_start(args, msg);
            vprintf(msg, args);
            va_end(args);
            printf("%s %s Line:%i\n", c_loc.file, c_loc.func, c_loc.loc);
            printf("\033[0m");
        }
	}
};

#endif
