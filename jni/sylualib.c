#include <stdarg.h>

#include "sylualib.h"
#define ERROR_TYPE luaL_error(L, "wrong result type");

void callLuaFunc(lua_State* L, const char* func, const char *sig, ...)
{
    va_list ap;
    int narg, nres;
    
    va_start(ap, sig);
    lua_getglobal(L, func);
    
    for(narg = 0; *sig; narg++)
    {
        luaL_checkstack(L, 1, "too many arguments");
        switch(*sig++)
        {
            case 'd':
                lua_pushnumber(L, va_arg(ap, double));
                break;
            case 'i':
                lua_pushinteger(L, va_arg(ap, int));
                break;
            case 's':
                lua_pushstring(L, va_arg(ap, char*));
                break;
			case 'p':
				lua_pushlightuserdata(L, va_arg(ap, void*));
				break;
            case '>':
                goto endargs;
            default:
                luaL_error(L, "incalid option (%c)", *(sig - 1));
        }
    }
endargs:
    nres = strlen(sig);
    if(lua_pcall(L, narg, nres, 0) != 0)
        luaL_error(L, "error calling '%s': %s", func, lua_tostring(L, -1));
    
    nres = -nres;
    while(*sig)
    {
        switch(*sig++)
        {
            case 'd':
                if(!lua_isnumber(L, nres))
                    ERROR_TYPE
                *va_arg(ap, double*) = lua_tonumber(L, nres);
                break;
            case 'i':
                if(!lua_isnumber(L, nres))
                    ERROR_TYPE
                *va_arg(ap, int*) = lua_tonumber(L, nres);
                break;
            case 's':
                if(!lua_isstring(L, nres))
                    ERROR_TYPE
                *va_arg(ap, const char **) = lua_tostring(L, nres);
                break;
            default:
                luaL_error(L, "invalid option (%c)", *(sig - 1));
        }
        nres++;
    }
    va_end(ap);
}

void addCFunc(lua_State* L, lua_CFunction fun, char* name)
{
    lua_pushcfunction(L, fun);
    lua_setglobal(L, name);
}

void* lua_getTableValue(lua_State* L, char* table, char* key, int type)
{
	lua_getglobal(L, table);
	lua_pushstring(L, key);
	lua_gettable(L, -2);
	void* re = NULL;
	if(type == LUA_TYPE_NUMBER)
	{
		re = malloc(sizeof(int) * 2);
		int t = lua_tonumber(L, -1);
		if(t == 0) return NULL;
		memcpy(re, &t, sizeof(t));
	}
	else if(type == LUA_TYPE_STRING)
	{
		const char* t = lua_tostring(L, -1);
		if(t == NULL) return NULL;
		re = malloc(strlen(t) + 1);
		memcpy(re, t, strlen(t));
	}
	return re;
}

