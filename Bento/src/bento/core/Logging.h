#pragma once

#ifdef ENABLE_TRACE
	#ifdef _MSC_VER
		#include <windows.h>
		#include <sstream>
		#include <stdio.h>
		
	#define TRACE(x)												\
			do 														\
			{  std::stringstream s;  s << (x);						\
				OutputDebugString(s.str().c_str());					\
			} while(0)

	#define PRINTF(STRING,...)											\
		{																\
			char buffer[512];											\
			sprintf_s(buffer, 511, (STRING), __VA_ARGS__);				\
			OutputDebugString(buffer);									\
		}
	#else												   
		#include <iostream>
		
		#define TRACE(x) std::cout << (x);
		#define PRINTF(STRING,...) printf(x, __VA_ARGS__);
	#endif
#else
	#define TRACE(x);
	#define PRINTF(x,...);
#endif
