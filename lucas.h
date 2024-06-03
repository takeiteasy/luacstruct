#ifndef _LUCAS_H
#define _LUCAS_H
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __has_include
#if __has_include("lucas_types.h")
#include "lucas_types.h"
#endif
#else
#include "lucas_types.h"
#endif
#include "minilua.h"
#include "luacstruct.h"

#ifdef __cplusplus
}
#endif
#endif // _LUCAS_H
