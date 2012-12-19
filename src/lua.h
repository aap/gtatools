#ifndef LUA_H
#define LUA_H

extern "C" {
	#include <lua5.1/lua.h>
	#include <lua5.1/lauxlib.h>
	#include <lua5.1/lualib.h>
}

void luaInterpreter(void);
void luaInit(void);

#endif
