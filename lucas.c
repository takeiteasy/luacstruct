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
 
#define LUA_IMPL
#include "lucas.h"

void LucasPrintStackAt(lua_State *L, int idx) {
    int t = lua_type(L, idx);
    switch (t) {
        case LUA_TSTRING:
            printf("(string): `%s'", lua_tostring(L, idx));
            break;
        case LUA_TBOOLEAN:
            printf("(boolean): %s", lua_toboolean(L, idx) ? "true" : "false");
            break;
        case LUA_TNUMBER:
            if (lua_isnumber(L, idx))
                printf("(number): %g",  lua_tonumber(L, idx));
            else
                printf("(integer): %lld",  lua_tointeger(L, idx));
            break;
        case LUA_TTABLE:
            printf("(table):\n");
            lua_settop(L, idx);
            LucasDumpTable(L);
            break;
        default:;
            printf("(%s): %p\n", lua_typename(L, t), lua_topointer(L, idx));
            break;
    }
}

int LucasDumpTable(lua_State* L) {
    if (!lua_istable(L, -1))
        luaL_error(L, "Expected a table at the top of the stack");
    
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        if (lua_type(L, -2) == LUA_TSTRING)
            printf("%s", lua_tostring(L, -2));
        else
            LucasPrintStackAt(L, -2);
        if (lua_type(L, -1) == LUA_TTABLE) {
            printf("\n");
            LucasDumpTable(L);
        } else {
            printf(" -- ");
            LucasPrintStackAt(L, -1);
            printf("\n");
        }
        lua_pop(L, 1);
    }
    return 0;
}

int LuaDumpStack(lua_State* L) {
    printf("--------------- LUA STACK DUMP ---------------\n");
    int top = lua_gettop(L);
    for (int i = top; i; --i) {
        printf("%d%s: ", i, i == top ? " (top)" : "");
        LucasPrintStackAt(L, i);
        if (i > 1)
            printf("\n");
    }
    printf("--------------- END STACK DUMP ---------------\n");
    return 0;
}

void LucasPushType(lua_State *L, LucasType *val) {
    switch (val->type) {
        case LUA_TNIL:
            lua_pushnil(L);
            break;
        case LUA_TBOOLEAN:
            lua_pushboolean(L, (int)val->data.integer);
            break;
        case LUA_TINTEGER:
            lua_pushinteger(L, val->data.integer);
            break;
        case LUA_TNUMBER:
            lua_pushnumber(L, val->data.number);
            break;
        case LUA_TSTRING:
            lua_pushstring(L, val->data.string);
            break;
        case LUA_TTABLE:
            lua_newtable(L);
            LucasTable *cursor = val->data.table;
            while (cursor) {
                lua_pushstring(L, cursor->key);
                LucasPushType(L, cursor->value);
                lua_settable(L, -3);
                cursor = cursor->next;
            }
            break;
        case LUA_TFUNCTION:
            lua_pushcfunction(L, (lua_CFunction)val->data.userdata);
            break;
        case LUA_TLIGHTUSERDATA:
        case LUA_TUSERDATA:
            lua_pushlightuserdata(L, val->data.userdata);
            break;
        case LUA_TTHREAD:
            lua_pushthread((lua_State*)val->data.userdata);
            break;
        default:
            luaL_error(L, "Unexpected type `%s`", lua_typename(L, val->type));
    }
}

static void PopulateLucasTable(lua_State *L, int idx, LucasTable **out) {
    LucasTable *head = malloc(sizeof(LucasTable));
    memset(head, 0, sizeof(LucasTable));
    LucasTable *cursor = NULL;
    
    assert(lua_istable(L, idx));
    lua_pushnil(L);
    while (lua_next(L, idx-1) != 0) {
        if (!cursor)
            cursor = head;
        else {
            LucasTable *new = malloc(sizeof(LucasTable));
            memset(new, 0, sizeof(LucasTable));
            cursor->next = new;
            cursor = new;
        }
        
        cursor->key = luaL_checkstring(L, idx-1);
        cursor->value = LucasGetType(L, idx);
        lua_pop(L, 1);
    }
    
    *out = head;
}

LucasType* LucasGetType(lua_State *L, int idx) {
    assert(lua_gettop(L) >= idx);
    LucasType *result = malloc(sizeof(LucasType));
    switch (result->type = lua_type(L, idx)) {
        case LUA_TNIL:
            result->data.userdata = NULL;
            break;
        case LUA_TBOOLEAN:
            result->data.integer = lua_toboolean(L, idx);
            break;
        case LUA_TNUMBER:
            if (lua_isinteger(L, idx)) {
                result->type = LUA_TINTEGER;
                result->data.integer = lua_tointeger(L, idx);
            } else
                result->data.number = lua_tonumber(L, idx);
            break;
        case LUA_TSTRING:
            result->data.string = lua_tostring(L, idx);
            break;
        case LUA_TTABLE: {
            PopulateLucasTable(L, idx, &result->data.table);
            break;
        }
        case LUA_TFUNCTION:
            result->data.userdata = (void*)lua_tocfunction(L, idx);
            break;
        case LUA_TLIGHTUSERDATA:
        case LUA_TUSERDATA:
            result->data.userdata = (void*)lua_topointer(L, idx);
            break;
        case LUA_TTHREAD:
            result->data.userdata = (void*)lua_tothread(L, idx);
            break;
        default:
            luaL_error(L, "Unexpected type `%s`", lua_typename(L, result->type));
    }
    return result;
}

void LucasFreeType(LucasType *val) {
    if (val) {
        switch (val->type) {
            case LUA_TSTRING:
                free((void*)val->data.string);
                break;
            case LUA_TTABLE:
                // TODO: Recursive free table
                break;
        }
        free(val);
    }
}
