#include <cstdio>
#include <readline/readline.h>
#include <readline/history.h>

#include <sys/poll.h>
#include <unistd.h>

#include "gta.h"
#include "lua.h"
#include "gl.h"
#include "camera.h"
#include "objects.h"
#include "world.h"
#include "renderer.h"
#include "jobqueue.h"
#include "drawable.h"
#include "animation.h"
#include "timecycle.h"

using namespace std;

int gettop(lua_State *L);
void registerGeneral(lua_State *L);
void registerCamera(lua_State *L);
void registerWorld(lua_State *L);
void registerGl(lua_State *L);
void registerTime(lua_State *L);

static lua_State *L = 0;
static string prompt;

void handleLine(char *s)
{
	static string line;
	static string statement = "";
	if (!s) {
		if (prompt == ">> ") {
			statement = "";
			prompt = "gta> ";
			cout << endl;
		} else {
			exitprog();
		}
		return;
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
	rl_callback_handler_install(prompt.c_str(), handleLine);
}

void luaInit(void)
{
//	while (L == 0);
	registerCamera(L);
	registerWorld(L);
	registerGl(L);
	registerTime(L);
	registerGeneral(L);

	luaL_dofile(L, "rc.lua");
	if (game == GTA3)
		luaL_dostring(L, "game = GTA3");
	else if (game == GTAVC)
		luaL_dostring(L, "game = GTAVC");
	else if (game == GTASA)
		luaL_dostring(L, "game = GTASA");

	luaL_dofile(L, "init.lua");
}

void luaInterpreter(void)
{
	struct pollfd fds;
	fds.fd = 0;
	fds.events = POLLIN;

	prompt = "gta> ";
	L = lua_open();
	luaL_openlibs(L);

	lua_register(L, "gettop", gettop);

	luaInit();

	rl_callback_handler_install(prompt.c_str(), handleLine);
	while (running)
		if(poll(&fds, 1, 500) == 1)
			rl_callback_read_char();
	lua_close(L);
}

int gettop(lua_State *L)
{
	lua_pushnumber(L, lua_gettop(L));
	return 1;
}

/*
 * General
 */

int setMixedAnim(lua_State *L)
{
	string arg1 = luaL_checkstring(L, 1);
	string arg2 = luaL_checkstring(L, 2);
	float arg3 = luaL_checknumber(L, 3);
	int ind1 = -1, ind2 = -1;
	for (size_t i = 0; i < anpk.animList.size(); i++) {
		if (anpk.animList[i].name == arg1)
			ind1 = i;
		if (anpk.animList[i].name == arg2)
			ind2 = i;
	}
	if (ind1 != -1 && ind2 != -1)
		drawable.attachMixedAnim(&anpk.animList[ind1],
		                         &anpk.animList[ind2],
		                         arg3);
	return 0;
}

int playerSetAnim(lua_State *L)
{
	string arg = luaL_checkstring(L, 1);
	player->setAnim(arg);
	return 0;
}

int setAnim(lua_State *L)
{
	string arg = luaL_checkstring(L, 1);
	for (size_t i = 0; i < anpk.animList.size(); i++) {
		if (anpk.animList[i].name == arg) {
			drawable.attachAnim(&anpk.animList[i]);
			break;
		}
	}
	return 0;
}

int setOvrAnim(lua_State *L)
{
	string arg = luaL_checkstring(L, 1);
	for (size_t i = 0; i < anpk.animList.size(); i++) {
		if (anpk.animList[i].name == arg) {
			drawable.attachOverrideAnim(&anpk.animList[i]);
			break;
		}
	}
	return 0;
}

int listAnims(lua_State *)
{
	for (size_t i = 0; i < anpk.animList.size(); i++)
		cout << anpk.animList[i].name << endl;
	return 0;
}

int resetDrawable(lua_State *)
{
	drawable.resetFrames();
	return 0;
}

int luausleep(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	usleep(i);
	return 0;
}

int playerLoad(lua_State *L)
{
	string arg1 = luaL_checkstring(L, 1);
	string arg2 = luaL_checkstring(L, 2);
	player->modelName = arg1;
	player->textureName = arg2;
//	player->loadSynch();
	player->load();
	return 0;
} 

int playerSetFrameTrans(lua_State *L)
{
	string name = luaL_checkstring(L, 1);
	int trans = luaL_checkinteger(L, 2);
	Frame *f = player->drawable->getFrame(name);
	if (f != 0)
		f->dotransform = trans;
	return 0;
}

void registerGeneral(lua_State *L)
{
	lua_register(L, "setAnim", setAnim);
	lua_register(L, "setOvrAnim", setOvrAnim);
	lua_register(L, "setMixedAnim", setMixedAnim);
	lua_register(L, "listAnims", listAnims);
	lua_register(L, "resetDrawable", resetDrawable);
	lua_register(L, "usleep", luausleep);

	lua_register(L, "playerLoad", playerLoad);
	lua_register(L, "playerSetAnim", playerSetAnim);
	lua_register(L, "playerSetFrameTrans", playerSetFrameTrans);
}

/*
 * Time
 */

int timeSetHour(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	timeCycle.setHour(i);
	return 0;
}

int timeGetHour(lua_State *L)
{
	lua_pushnumber(L, timeCycle.getHour());
	return 1;
}

int timeSetMinute(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	timeCycle.setMinute(i);
	return 0;
}

int timeGetMinute(lua_State *L)
{
	lua_pushnumber(L, timeCycle.getMinute());
	return 1;
}

int timeSetCurrentWeather(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	timeCycle.setCurrentWeather(i);
	return 0;
}

int timeGetCurrentWeather(lua_State *L)
{
	lua_pushnumber(L, timeCycle.getCurrentWeather());
	return 1;
}

void registerTime(lua_State *L)
{
	lua_register(L, "__timeSetHour", timeSetHour);
	lua_register(L, "__timeGetHour", timeGetHour);
	lua_register(L, "__timeSetMinute", timeSetMinute);
	lua_register(L, "__timeGetMinute", timeGetMinute);
	lua_register(L, "__timeSetCurrentWeather", timeSetCurrentWeather);
	lua_register(L, "__timeGetCurrentWeather", timeGetCurrentWeather);
}

/*
 * Camera
 */

/*
int cameraPanLR(lua_State *L)
{
	float d = luaL_checknumber(L, 1);
	cam->panLR(d);
	return 0;
}

int cameraPanUD(lua_State *L)
{
	float d = luaL_checknumber(L, 1);
	cam->panUD(d);
	return 0;
}

int cameraTurnLR(lua_State *L)
{
	float phi = luaL_checknumber(L, 1);
	cam->turnLR(phi);
	return 0;
}

int cameraTurnUD(lua_State *L)
{
	float phi = luaL_checknumber(L, 1);
	cam->turnUD(phi);
	return 0;
}

int cameraMoveInOut(lua_State *L)
{
	float d = luaL_checknumber(L, 1);
	cam->moveInOut(d);
	return 0;
}

int cameraSetPitch(lua_State *L)
{
	float pitch = luaL_checknumber(L, 1);
	cam->setPitch(pitch);
	return 0;
}

int cameraSetYaw(lua_State *L)
{
	float yaw = luaL_checknumber(L, 1);
	cam->setYaw(yaw);
	return 0;
}

int cameraSetDistance(lua_State *L)
{
	float d = luaL_checknumber(L, 1);
	cam->setDistance(d);
	return 0;
}

int cameraSetTarget(lua_State *L)
{
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	float z = luaL_checknumber(L, 3);
	cam->setTarget(quat(x, y, z));
	return 0;
}

int cameraSetFov(lua_State *L)
{
	float fov = luaL_checknumber(L, 1);
	cam->setFov(fov);
	return 0;
}

int cameraGetPitch(lua_State *L)
{
	lua_pushnumber(L, cam->getPitch());
	return 1;
}

int cameraGetYaw(lua_State *L)
{
	lua_pushnumber(L, cam->getYaw());
	return 1;
}

int cameraGetDistance(lua_State *L)
{
	lua_pushnumber(L, cam->getDistance());
	return 1;
}

int cameraGetTarget(lua_State *L)
{
	quat target = cam->getTarget();
	lua_pushnumber(L, target.x);
	lua_pushnumber(L, target.y);
	lua_pushnumber(L, target.z);
	return 3;
}

int cameraGetFov(lua_State *L)
{
	lua_pushnumber(L, cam->getFov());
	return 1;
}

int cameraGetPosition(lua_State *L)
{
	quat position = cam->getPosition();
	lua_pushnumber(L, position.x);
	lua_pushnumber(L, position.y);
	lua_pushnumber(L, position.z);
	return 3;
}

int cameraLock(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	if (i)
		cam->lock(&player->frm);
	else
		cam->lock(0);
	return 0;
}

int cameraPrint(lua_State *)
{
	quat target = cam->getTarget();
	cout << "cam.setTarget(" << target.x << "," << target.y
	     << "," << target.z << ")\n";
	cout << "cam.setDistance(" << cam->getDistance() << ")\n";
	cout << "cam.setYaw(" << cam->getYaw() << ")\n";
	cout << "cam.setPitch(" << cam->getPitch() << ")\n";
	return 0;
}
*/

int cameraSetTarget(lua_State *L)
{
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	float z = luaL_checknumber(L, 3);
	cam->setTarget(quat(x, y, z));
	return 0;
}

int cameraGetTarget(lua_State *L)
{
	quat q = cam->getTarget();
	lua_pushnumber(L, q.x);
	lua_pushnumber(L, q.y);
	lua_pushnumber(L, q.z);
	return 3;
}

int cameraSetPosition(lua_State *L)
{
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	float z = luaL_checknumber(L, 3);
	cam->setPosition(quat(x, y, z));
	return 0;
}

int cameraGetPosition(lua_State *L)
{
	quat q = cam->getPosition();
	lua_pushnumber(L, q.x);
	lua_pushnumber(L, q.y);
	lua_pushnumber(L, q.z);
	return 3;
}

int cameraTurn(lua_State *L)
{
	float a = luaL_checknumber(L, 1);
	float b = luaL_checknumber(L, 2);
	cam->turn(a, b);
	return 0;
}

int cameraOrbit(lua_State *L)
{
	float a = luaL_checknumber(L, 1);
	float b = luaL_checknumber(L, 2);
	cam->orbit(a, b);
	return 0;
}

int cameraDolly(lua_State *L)
{
	float a = luaL_checknumber(L, 1);
	cam->dolly(a);
	return 0;
}

int cameraZoom(lua_State *L)
{
	float a = luaL_checknumber(L, 1);
	cam->zoom(a);
	return 0;
}

int cameraSetFov(lua_State *L)
{
	float fov = luaL_checknumber(L, 1);
	cam->setFov(fov);
	return 0;
}

int cameraGetFov(lua_State *L)
{
	lua_pushnumber(L, cam->getFov());
	return 1;
}

int cameraLock(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	if (i)
		cam->lock(&player->frm);
	else
		cam->lock(0);
	return 0;
}

int cameraPrint(lua_State *)
{
	quat position = cam->getPosition();
	quat target = cam->getTarget();
	cout << "cam.setPosition(" << position.x << "," << position.y
	     << "," << position.z << ")\n";
	cout << "cam.setTarget(" << target.x << "," << target.y
	     << "," << target.z << ")\n";
	return 0;
}

void registerCamera(lua_State *L)
{
/*
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
	lua_register(L, "__cameraLock", cameraLock);
	lua_register(L, "__cameraPrint", cameraPrint);
*/

	lua_register(L, "__cameraSetPosition", cameraSetPosition);
	lua_register(L, "__cameraGetPosition", cameraGetPosition);
	lua_register(L, "__cameraSetTarget", cameraSetTarget);
	lua_register(L, "__cameraGetTarget", cameraGetTarget);
	lua_register(L, "__cameraTurn", cameraTurn);
	lua_register(L, "__cameraOrbit", cameraOrbit);
	lua_register(L, "__cameraDolly", cameraDolly);
	lua_register(L, "__cameraZoom", cameraZoom);

	lua_register(L, "__cameraSetFov", cameraSetFov);
	lua_register(L, "__cameraGetFov", cameraGetFov);
	lua_register(L, "__cameraLock", cameraLock);
	lua_register(L, "__cameraPrint", cameraPrint);
}

/*
 * World
 */

int worldSetInterior(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	world->setInterior(i);
	return 0;
}

int worldGetInterior(lua_State *L)
{
	lua_pushinteger(L, world->getInterior());
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
	renderer->doTextures = i;
	return 0;
}

int rendererGetDoTextures(lua_State *L)
{
	lua_pushinteger(L, renderer->doTextures);
	return 1;
}

int rendererSetDoZones(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	renderer->doZones = i;
	return 0;
}

int rendererGetDoZones(lua_State *L)
{
	lua_pushinteger(L, renderer->doZones);
	return 1;
}

int rendererSetDoFog(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	renderer->doFog = i;
	return 0;
}

int rendererGetDoFog(lua_State *L)
{
	lua_pushinteger(L, renderer->doFog);
	return 1;
}

int rendererSetDoVertexColors(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	renderer->doVertexColors = i;
	return 0;
}

int rendererGetDoVertexColors(lua_State *L)
{
	lua_pushinteger(L, renderer->doVertexColors);
	return 1;
}

int rendererSetDoCol(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	renderer->doCol = i;
	return 0;
}

int rendererGetDoCol(lua_State *L)
{
	lua_pushinteger(L, renderer->doCol);
	return 1;
}

int rendererSetDoTrails(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	renderer->doTrails = i;
	return 0;
}

int rendererGetDoTrails(lua_State *L)
{
	lua_pushinteger(L, renderer->doTrails);
	return 1;
}

int rendererSetDoBFC(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	renderer->doBFC = i;
	return 0;
}

int rendererGetDoBFC(lua_State *L)
{
	lua_pushinteger(L, renderer->doBFC);
	return 1;
}

int rendererSetLodMult(lua_State *L)
{
	float f = luaL_checknumber(L, 1);
	renderer->lodMult = f;
	return 0;
}

int rendererGetLodMult(lua_State *L)
{
	lua_pushnumber(L, renderer->lodMult);
	return 1;
}

int rendererFreeze(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	renderer->freeze = (i != 0);
	return 0;
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
	lua_register(L, "__rendererFreeze", rendererFreeze);
}
