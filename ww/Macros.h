/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "yasli/Assert.h"

#ifdef _WIN64
#include <emmintrin.h>
#endif

#define FOR_EACH(container, it) for( it = (container).begin(); it != (container).end(); ++it )

template<class T, size_t Len>
char (&globalArrayLenHelper(const T(&)[Len]))[Len];
#define ARRAY_LEN(arr) sizeof(globalArrayLenHelper(arr)) 

#ifndef NDEBUG
# define WW_VERIFY(x) YASLI_ASSERT(x)
#else
# define WW_VERIFY(x) (x)
#endif

#undef min
#undef max

namespace ww {

inline int min(int x,int y){ return x < y ? x : y; }
inline float min(float x,float y){ return x < y ? x : y; }
inline double min(double x,double y){ return x < y ? x : y; }

inline int max(int x,int y){ return x > y ? x : y; }
inline float max(float x,float y){ return x > y ? x : y; }
inline double max(double x,double y){ return x > y ? x : y; }

#pragma warning (disable : 4244)

template<class T, class T1, class T2> 
inline T clamp(const T& x, const T1& xmin, const T2& xmax)
{
  if(x < xmin)
    return xmin;
  if(x > xmax)
    return xmax;
  return x;
}

inline int round(double x)
{
    int a;
#ifdef _WIN64
	return _mm_cvtsd_si32(_mm_load_sd(&x));
#else
    _asm {
        fld x
        fistp dword ptr a
    }
#endif
    return a;
}

inline int round(float x)
{
#ifdef _WIN64
	return _mm_cvtt_ss2si(_mm_load_ss(&x));
#else
    int a;
    _asm {
        fld x
        fistp dword ptr a
    }
    return a;
#endif
}

}
