/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once 
#include "yasli/Config.h"

#ifndef YASLI_ASSERT_DEFINED
#include <stdio.h>

#ifdef _MSC_VER
#define YASLI_DEBUG_BREAK __debugbreak()
#elif defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#define YASLI_DEBUG_BREAK EM_ASM(console.error("debug-break"))
#else
#define YASLI_DEBUG_BREAK __builtin_trap()
#endif

namespace yasli{
void setTestMode(bool interactive);
typedef void (*LogHandler)(const char*);
void setLogHandler(LogHandler handler);
}


#ifndef NDEBUG
namespace yasli{
void setInteractiveAssertion(bool interactive);
bool assertionDialog(const char* function, const char* fileName, int line, const char* expr, const char* str, ...);
inline bool assertionDialog(const char* function, const char* fileName, int line, const char* expr) { return assertionDialog(function, fileName, line, expr, ""); }
}
#ifdef _MSC_VER
# define YASLI_ASSERT(expr, ...) ((expr) || (yasli::assertionDialog(__FUNCTION__, __FILE__, __LINE__, #expr, __VA_ARGS__) ? YASLI_DEBUG_BREAK, false : false))
#else
// gcc doesn't remove trailing comma when __VA_ARGS__ is empty, but one can workaround this with ## prefix
#ifdef __clang__
# pragma clang system_header // to disable -Wunused-value
#endif
# define YASLI_ASSERT(expr, ...) ((expr) || (yasli::assertionDialog(__FUNCTION__, __FILE__, __LINE__, #expr, ##__VA_ARGS__) ? YASLI_DEBUG_BREAK, false : false))
#endif
#define YASLI_CHECK YASLI_ASSERT
#else
#define YASLI_ASSERT(expr, ...) (1, true)
#define YASLI_CHECK(expr, ...) (expr)
namespace yasli {
  inline bool assertionDialog(const char* function, const char* fileName, int line, const char* expr) { return false; }
}
#endif

// use YASLI_CHECK instead
#define YASLI_ESCAPE(x, action) if(!(x)) { YASLI_ASSERT(0 && #x); action; } 

#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif

#endif
