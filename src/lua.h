#ifndef LUA_H
#define LUA_H

extern "C" {
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>
}

void LuaInterpreter(void);

#endif
