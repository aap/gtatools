#include <cstdio>
#include <readline/readline.h>
#include <readline/history.h>

#include "gta.h"
#include "gl.h"
#include "camera.h"
#include "world.h"

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
/*
		cout << prompt;
		getline(cin, line);

		if (cin.eof()) {
			if (prompt == ">> ") {
				statement = "";
				prompt = "gta> ";
				cin.clear();
				cout << endl;
			} else {
				running = false;
			}
			continue;
		}
*/

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
 * Gl
 */

int glSetDoTextures(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	gl::doTextures = i;
	return 0;
}

int glGetDoTextures(lua_State *L)
{
	lua_pushinteger(L, gl::doTextures);
	return 1;
}

int glSetDoZones(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	gl::doZones = i;
	return 0;
}

int glGetDoZones(lua_State *L)
{
	lua_pushinteger(L, gl::doZones);
	return 1;
}

int glSetDoFog(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	gl::doFog = i;
	return 0;
}

int glGetDoFog(lua_State *L)
{
	lua_pushinteger(L, gl::doFog);
	return 1;
}

int glSetDoVertexColors(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	gl::doVertexColors = i;
	return 0;
}

int glGetDoVertexColors(lua_State *L)
{
	lua_pushinteger(L, gl::doVertexColors);
	return 1;
}

int glSetDoCol(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	gl::doCol = i;
	return 0;
}

int glGetDoCol(lua_State *L)
{
	lua_pushinteger(L, gl::doCol);
	return 1;
}

void registerGl(lua_State *L)
{
	lua_register(L, "__glSetDoTextures", glSetDoTextures);
	lua_register(L, "__glGetDoTextures", glGetDoTextures);
	lua_register(L, "__glSetDoZones", glSetDoZones);
	lua_register(L, "__glGetDoZones", glGetDoZones);
	lua_register(L, "__glSetDoFog", glSetDoFog);
	lua_register(L, "__glGetDoFog", glGetDoFog);
	lua_register(L, "__glSetDoVertexColors", glSetDoVertexColors);
	lua_register(L, "__glGetDoVertexColors", glGetDoVertexColors);
	lua_register(L, "__glSetDoCol", glSetDoCol);
	lua_register(L, "__glGetDoCol", glGetDoCol);
}
