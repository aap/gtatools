#include <GL/glew.h>
#include <GL/glfw.h>
#include "gl.h"
#include "gta.h"
#include "camera.h"
#include "jobqueue.h"
#include "renderer.h"
#include "objects.h"
#include "drawable.h"

using namespace std;

namespace gl {

static int ox, oy, mx, my;
static bool isLDown, isMDown, isRDown;
static bool isShiftDown, isCtrlDown, isAltDown;
static uint lastSelected;

void handleKeyboardInput(void)
{
	const float dist = 5.0f;

	isShiftDown = (glfwGetKey(GLFW_KEY_LSHIFT) == GLFW_PRESS) ?
		true : false;
	isCtrlDown = (glfwGetKey(GLFW_KEY_LCTRL) == GLFW_PRESS) ?
		true : false;
	isAltDown = (glfwGetKey(GLFW_KEY_LALT) == GLFW_PRESS) ?
		true : false;

	if (glfwGetKey('Y')) {
		if (isShiftDown)
			drawable.setTime(drawable.getTime()-0.033333/2.0);
		else
			drawable.setTime(drawable.getTime()+0.033333/2.0);
	}

	if (glfwGetKey('U')) {
		if (isShiftDown)
			drawable.setOvrTime(drawable.getOvrTime()-0.033333/2.0);
		else
			drawable.setOvrTime(drawable.getOvrTime()+0.033333/2.0);
	}

	if (glfwGetKey('W')) {
		if (isShiftDown)
			cam->moveInOut(dist);
		else
			cam->setDistance(cam->getDistance()-dist);
	}

	if (glfwGetKey('S')) {
		if (isShiftDown)
			cam->moveInOut(-dist);
		else
			cam->setDistance(cam->getDistance()+dist);
	}

}

void keypress(int key, int state)
{
	if (state != GLFW_PRESS)
		return;

	switch (key) {
	case 'Q':
	case GLFW_KEY_ESC:
		exitprog();
		break;
	case 'B':
		WorldObject *op;
		op = static_cast<WorldObject*>(objectList->get(
			world->getInstance(lastSelected)->id));
		if (isShiftDown)
			op->BSvisible = false;
		else
			op->BSvisible = true;
		break;
	case 'P':
		if (isShiftDown) {
			if (lastSelected == 0)
				break;
			uint id = world->getInstance(lastSelected)->id;
			Model *mp = objectList->get(id);
			mp->drawable->printFrames(0,0);
		} else
			drawable.printFrames(0, 0);
		break;
	case 'I':
		if (lastSelected == 0)
			break;
		world->getInstance(lastSelected)->printInfo();
		break;
	default:
		break;
	}
}

void mouseButton(int button, int state)
{
	int x, y;
	glfwGetMousePos(&x, &y);
	if (state == GLFW_PRESS) {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			ox = mx = x;
			oy = my = y;
			isLDown = true;
		}
		if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
			ox = mx = x;
			oy = my = y;
			isMDown = true;
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			ox = mx = x;
			oy = my = y;
			isRDown = true;
		}
	} else if (state == GLFW_RELEASE) {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			isLDown = false;
			if (ox - mx == 0 && oy - my == 0) {
				cout << x << " " << y << endl;
				// read clicked object's index
				union {
					char bytes[4];
					int integer;
				} stencil;
				// fewer passes would probably be enough
				stencilShift = 0;
				renderer->renderScene();
				glReadPixels(x, height - y - 1, 1, 1,
				             GL_STENCIL_INDEX,
				             GL_UNSIGNED_INT, stencil.bytes);
				stencilShift = 8;
				renderer->renderScene();
				glReadPixels(x, height - y - 1, 1, 1,
				             GL_STENCIL_INDEX,
				             GL_UNSIGNED_INT, stencil.bytes+1);
				stencilShift = 16;
				renderer->renderScene();
				glReadPixels(x, height - y - 1, 1, 1,
				             GL_STENCIL_INDEX,
				             GL_UNSIGNED_INT, stencil.bytes+2);
				stencilShift = 24;
				renderer->renderScene();
				glReadPixels(x, height - y - 1, 1, 1,
				             GL_STENCIL_INDEX,
				             GL_UNSIGNED_INT, stencil.bytes+3);
				lastSelected = stencil.integer;
			}
		}
		if (button == GLFW_MOUSE_BUTTON_MIDDLE)
			isMDown = false;
		if (button == GLFW_MOUSE_BUTTON_RIGHT)
			isRDown = false;
	}
}

void mouseMotion(int x, int y)
{
	GLfloat dx, dy;

	ox = mx;
	oy = my;
	mx = x;
	my = y;

	dx = float(ox - mx) / float(width);
	dy = float(oy - my) / float(height);
	if (!isShiftDown) {
		if (isLDown) {
			cam->setYaw(cam->getYaw()+dx*2.0f);
			cam->setPitch(cam->getPitch()-dy*2.0f);
		}
		if (isMDown) {
			cam->panLR(-dx*8.0f);
			cam->panUD(-dy*8.0f);
		}
		if (isRDown) {
			cam->setDistance(cam->getDistance()+dx*12.0f);
		}
	} else if (isShiftDown) {
		if (isLDown) {
			cam->turnLR(dx*2.0f);
			cam->turnUD(-dy*2.0f);
		}
		if (isMDown) {
		}
		if (isRDown) {
		}
	}
}

void handleJoystickInput(int joy)
{
	int nAxes = glfwGetJoystickParam(GLFW_JOYSTICK_1, GLFW_AXES);
	int nButtons = glfwGetJoystickParam(GLFW_JOYSTICK_1, GLFW_BUTTONS);
	static float *axes = 0;
	static uchar *buttons = 0;
	if (!axes) {
		axes = new float[nAxes];
		buttons = new uchar[nButtons];
	}
	glfwGetJoystickButtons(joy, buttons, nButtons); 
	glfwGetJoystickPos(joy, axes, nAxes); 

	float accel = 1.0f;
	if (buttons[5])
		accel = 2.0f;
	if (buttons[4])
		accel = 0.5f;
	cam->moveInOut(-axes[2]*accel);
	cam->turnLR(-axes[0]/10.0f);
	cam->turnUD(axes[1]/10.0f);
}



/*
void keypress(uchar key, int x, int y)
{
	char buf[20];
	WorldObject *op;
//	Instance *ip;
	quat campos;
	static float dist = 10.0f;

	switch (key) {
	case 'c':
		world.cleanUp();
		break;
	case 'y':
		drawable.setTime(drawable.getTime()+0.033333);
		break;
	case 'Y':
		drawable.setTime(drawable.getTime()-0.033333);
		break;
	case 'W':
		cam.moveInOut(dist);
		break;
	case 'w':
		cam.setDistance(cam.getDistance()-dist);
		break;
	case 'S':
		cam.moveInOut(-dist);
		break;
	case 's':
		cam.setDistance(cam.getDistance()+dist);
		break;
	case 'd':
		op = static_cast<WorldObject*>(objectList.get(
			world.getInstance(lastSelected)->id));
		op->drawable->dumpClump(false);
		break;
	case 'D':
		op = static_cast<WorldObject*>(objectList.get(
			world.getInstance(lastSelected)->id));
		op->drawable->dumpClump(true);
		break;
	case 'x':
		dist /= 2.0f;
		break;
	case 'X':
		dist *= 2.0f;
		break;
	case 'm':
		timeCycle.setMinute(timeCycle.getMinute()+1);
		sprintf(buf, "%02d:%02d", timeCycle.getHour(),
		                          timeCycle.getMinute());
		statusLine = buf;
		break;
	case 'M':
		timeCycle.setMinute(timeCycle.getMinute()-1);
		sprintf(buf, "%02d:%02d", timeCycle.getHour(),
		                          timeCycle.getMinute());
		statusLine = buf;
		break;
	case 'h':
		timeCycle.setHour(timeCycle.getHour()+1);
		sprintf(buf, "%02d:%02d", timeCycle.getHour(),
		                          timeCycle.getMinute());
		statusLine = buf;
		break;
	case 'H':
		timeCycle.setHour(timeCycle.getHour()-1);
		sprintf(buf, "%02d:%02d", timeCycle.getHour(),
		                          timeCycle.getMinute());
		statusLine = buf;
		break;
	case 'v':
		if (lastSelected == 0)
			break;
		world.getInstance(lastSelected)->isHidden = true;
		break;
	case 'V':
		if (lastSelected == 0)
			break;
		world.getInstance(lastSelected)->isHidden = false;
		break;
	case 'b':
		if (lastSelected == 0)
			break;
		op = static_cast<WorldObject*>(objectList.get(
			world.getInstance(lastSelected)->id));
		op->BSvisible = true;
		break;
	case 'B':
		if (lastSelected == 0)
			break;
		op = static_cast<WorldObject*>(objectList.get(
			world.getInstance(lastSelected)->id));
		op->BSvisible = false;
		break;
	case 'i':
		if (lastSelected == 0)
			break;
		world.getInstance(lastSelected)->printInfo();
		break;
	case 'p':
		drawable.printFrames(0, 0);
		break;
	case 'q':
	case 27:
		running = false;
		normalJobs.wakeUp();
		break;
	default:
		break;
	}
}

void keypressSpecial(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_LEFT:
		cam.setYaw(cam.getYaw()+0.1f);
		break;
	case GLUT_KEY_RIGHT:
		cam.setYaw(cam.getYaw()-0.1f);
		break;
	case GLUT_KEY_UP:
		cam.setPitch(cam.getPitch()-0.1f);
		break;
	case GLUT_KEY_DOWN:
		cam.setPitch(cam.getPitch()+0.1f);
		break;
	default:
		break;
	}
}
*/

}
