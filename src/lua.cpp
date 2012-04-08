#include <cstdio>
#include <readline/readline.h>
#include <readline/history.h>

#include "gta.h"
#include "gl.h"
#include "camera.h"
#include "world.h"
#include "renderer.h"

#include "lua.h"

using namespace std;

int gettop(lua_State *L);
void registerCamera(lua_State *L);
void registerWorld(lua_State *L);
void registerGl(lua_State *L);

void LuaInterpreter(void)
{
	string line;
	string statement = "";
	string prompt = "gta> ";
	lua_State *L = lua_open();
	luaL_openlibs(L);

	lua_register(L, "gettop", gettop);
	registerCamera(L);
	registerWorld(L);
	registerGl(L);

	luaL_dofile(L, "rc.lua");

	while (running) {
		char *s = readline(prompt.c_str());
		if (!s) {
			if (prompt == ">> ") {
				statement = "";
				prompt = "gta> ";
				cout << endl;
			} else {
				running = false;
			}
			continue;
		}
		add_history(s);
		line = s;

		statement += " " + line;

		// load the statement
		if (luaL_loadstring(L, statement.c_str()) != 0) {
			string error(lua_tostring(L, -1));
			if (error.substr(error.size()-7, 7) == "'<eof>'") {
				// get more input
				prompt = ">> ";
			} else {
				// syntax error
				cout << error << endl;
				statement = "";
				prompt = "gta> ";
			}
		// try to execute and catch errors
		} else {
			if (lua_pcall(L, 0, LUA_MULTRET, 0) != 0) {
				string error(lua_tostring(L, -1));
				cout << error << endl;
				lua_gc(L, LUA_GCCOLLECT, 0);
			}
			statement = "";
			prompt = "gta> ";
		}
	}
	lua_close(L);
}

int gettop(lua_State *L)
{
	lua_pushnumber(L, lua_gettop(L));
	return 1;
}

/*
 * Camera
 */

int cameraPanLR(lua_State *L)
{
	float d = luaL_checknumber(L, 1);
	cam.panLR(d);
	return 0;
}

int cameraPanUD(lua_State *L)
{
	float d = luaL_checknumber(L, 1);
	cam.panUD(d);
	return 0;
}

int cameraTurnLR(lua_State *L)
{
	float phi = luaL_checknumber(L, 1);
	cam.turnLR(phi);
	return 0;
}

int cameraTurnUD(lua_State *L)
{
	float phi = luaL_checknumber(L, 1);
	cam.turnUD(phi);
	return 0;
}

int cameraMoveInOut(lua_State *L)
{
	float d = luaL_checknumber(L, 1);
	cam.moveInOut(d);
	return 0;
}

int cameraSetPitch(lua_State *L)
{
	float pitch = luaL_checknumber(L, 1);
	cam.setPitch(pitch);
	return 0;
}

int cameraSetYaw(lua_State *L)
{
	float yaw = luaL_checknumber(L, 1);
	cam.setYaw(yaw);
	return 0;
}

int cameraSetDistance(lua_State *L)
{
	float d = luaL_checknumber(L, 1);
	cam.setDistance(d);
	return 0;
}

int cameraSetTarget(lua_State *L)
{
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	float z = luaL_checknumber(L, 3);
	cam.setTarget(quat(x, y, z));
	return 0;
}

int cameraSetFov(lua_State *L)
{
	float fov = luaL_checknumber(L, 1);
	cam.setFov(fov);
	return 0;
}

int cameraGetPitch(lua_State *L)
{
	lua_pushnumber(L, cam.getPitch());
	return 1;
}

int cameraGetYaw(lua_State *L)
{
	lua_pushnumber(L, cam.getYaw());
	return 1;
}

int cameraGetDistance(lua_State *L)
{
	lua_pushnumber(L, cam.getDistance());
	return 1;
}

int cameraGetTarget(lua_State *L)
{
	quat target = cam.getTarget();
	lua_pushnumber(L, target.x);
	lua_pushnumber(L, target.y);
	lua_pushnumber(L, target.z);
	return 3;
}

int cameraGetFov(lua_State *L)
{
	lua_pushnumber(L, cam.getFov());
	return 1;
}

int cameraGetPosition(lua_State *L)
{
	quat position = cam.getPosition();
	lua_pushnumber(L, position.x);
	lua_pushnumber(L, position.y);
	lua_pushnumber(L, position.z);
	return 3;
}

void registerCamera(lua_State *L)
{
	lua_register(L, "__cameraPanLR", cameraPanLR);
	lua_register(L, "__cameraPanUD", cameraPanUD);
	lua_register(L, "__cameraTurnLR", cameraTurnLR);
	lua_register(L, "__cameraTurnUD", cameraTurnUD);
	lua_register(L, "__cameraMoveInOut", cameraMoveInOut);
	lua_register(L, "__cameraSetPitch", cameraSetPitch);
	lua_register(L, "__cameraSetYaw", cameraSetYaw);
	lua_register(L, "__cameraSetDistance", cameraSetDistance);
	lua_register(L, "__cameraSetTarget", cameraSetTarget);
	lua_register(L, "__cameraSetFov", cameraSetFov);
	lua_register(L, "__cameraGetPitch", cameraGetPitch);
	lua_register(L, "__cameraGetYaw", cameraGetYaw);
	lua_register(L, "__cameraGetDistance", cameraGetDistance);
	lua_register(L, "__cameraGetTarget", cameraGetTarget);
	lua_register(L, "__cameraGetFov", cameraGetFov);
	lua_register(L, "__cameraGetPosition", cameraGetPosition);
}

/*
 * World
 */

int worldSetInterior(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	world.setInterior(i);
	return 0;
}

int worldGetInterior(lua_State *L)
{
	lua_pushinteger(L, world.getInterior());
	return 1;
}

void registerWorld(lua_State *L)
{
	lua_register(L, "__worldSetInterior", worldSetInterior);
	lua_register(L, "__worldGetInterior", worldGetInterior);
}

/*
 * Renderer
 */

int rendererSetDoTextures(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	renderer.doTextures = i;
	return 0;
}

int rendererGetDoTextures(lua_State *L)
{
	lua_pushinteger(L, renderer.doTextures);
	return 1;
}

int rendererSetDoZones(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	renderer.doZones = i;
	return 0;
}

int rendererGetDoZones(lua_State *L)
{
	lua_pushinteger(L, renderer.doZones);
	return 1;
}

int rendererSetDoFog(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	renderer.doFog = i;
	return 0;
}

int rendererGetDoFog(lua_State *L)
{
	lua_pushinteger(L, renderer.doFog);
	return 1;
}

int rendererSetDoVertexColors(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	renderer.doVertexColors = i;
	return 0;
}

int rendererGetDoVertexColors(lua_State *L)
{
	lua_pushinteger(L, renderer.doVertexColors);
	return 1;
}

int rendererSetDoCol(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	renderer.doCol = i;
	return 0;
}

int rendererGetDoCol(lua_State *L)
{
	lua_pushinteger(L, renderer.doCol);
	return 1;
}

int rendererSetDoTrails(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	renderer.doTrails = i;
	return 0;
}

int rendererGetDoTrails(lua_State *L)
{
	lua_pushinteger(L, renderer.doTrails);
	return 1;
}

int rendererSetDoBFC(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	renderer.doBFC = i;
	return 0;
}

int rendererGetDoBFC(lua_State *L)
{
	lua_pushinteger(L, renderer.doBFC);
	return 1;
}

int rendererSetLodMult(lua_State *L)
{
	float f = luaL_checknumber(L, 1);
	renderer.lodMult = f;
	return 0;
}

int rendererGetLodMult(lua_State *L)
{
	lua_pushnumber(L, renderer.lodMult);
	return 1;
}

void registerGl(lua_State *L)
{
	lua_register(L, "__rendererSetDoTextures", rendererSetDoTextures);
	lua_register(L, "__rendererGetDoTextures", rendererGetDoTextures);
	lua_register(L, "__rendererSetDoZones", rendererSetDoZones);
	lua_register(L, "__rendererGetDoZones", rendererGetDoZones);
	lua_register(L, "__rendererSetDoFog", rendererSetDoFog);
	lua_register(L, "__rendererGetDoFog", rendererGetDoFog);
	lua_register(L,"__rendererSetDoVertexColors",rendererSetDoVertexColors);
	lua_register(L,"__rendererGetDoVertexColors",rendererGetDoVertexColors);
	lua_register(L, "__rendererSetDoCol", rendererSetDoCol);
	lua_register(L, "__rendererGetDoCol", rendererGetDoCol);
	lua_register(L, "__rendererSetDoTrails", rendererSetDoTrails);
	lua_register(L, "__rendererGetDoTrails", rendererGetDoTrails);
	lua_register(L, "__rendererSetDoBFC", rendererSetDoBFC);
	lua_register(L, "__rendererGetDoBFC", rendererGetDoBFC);
	lua_register(L, "__rendererSetLodMult", rendererSetLodMult);
	lua_register(L, "__rendererGetLodMult", rendererGetLodMult);
}
