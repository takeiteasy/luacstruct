/* lucas.h -- Stretchy buffer implementation https://github.com/takeiteasy/luacstruct
 
 The MIT License (MIT)
 Copyright (c) 2024 George Watson
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

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

#define LUA_TINTEGER 9
#undef LUA_NUMTYPES
#define LUA_NUMTYPES 10

typedef struct LucasType LucasType;

typedef struct LucasTable {
    const char *key;
    LucasType *value;
    struct LucasTable *next;
} LucasTable;

struct LucasType {
    union {
        lua_Integer integer;
        lua_Number number;
        const char *string;
        void *userdata;
        LucasTable *table;
    } data;
    int type;
};

void LucasPushType(lua_State *L, LucasType *val);
LucasType* LucasGetType(lua_State *L, int idx);
void LucasFreeType(LucasType *val);

void LucasPrintStackAt(lua_State *L, int idx);
int LucasDumpTable(lua_State* L);
int LucasDumpStack(lua_State* L);

#ifdef __cplusplus
}
#endif
#endif // _LUCAS_H
