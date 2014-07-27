/*
Implementação do plugin FFT-Varmap.

Este plugin computa varmaps usando FFT.

(c) 2014, LPM/UFRGS, Péricles Lopes Machado
*/



#ifndef _WGSLIB_COMMON_H_
#define _WGSLIB_COMMON_H_

#include <string>

#if defined(_WIN32) || defined(WIN32)
  #ifdef LIB_STATIC
    #define WGSLIB_DECL
  #else
    #ifdef WGSLIB_EXPORT
      #define WGSLIB_DECL __declspec(dllexport)
    #else
      #define WGSLIB_DECL __declspec(dllimport)
    #endif
  #endif
#else
    #define WGSLIB_DECL
#endif

#include <cstring>

namespace LPM_UFRGS {
	inline std::string to_string(int i)
	{
		char buffer[100];
		sprintf(buffer, "%07d", i);
		
		return buffer;
	}
}

struct Point3D {
	double x, y, z;
	Point3D(double x = 0, double y = 0, double z = 0)
		: x(x), y(y), z(z)
	{}
};


#endif // PLUGINS_LPM_UFRGS_COMMON_H_
