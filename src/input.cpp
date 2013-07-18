#include <GL/glew.h>
#include <GL/glfw.h>
#include "gl.h"
#include "gta.h"
#include "world.h"
#include "enex.h"
#include "camera.h"
#include "jobqueue.h"
#include "renderer.h"
#include "objects.h"
#include "drawable.h"
#include "timecycle.h"
#include "phys.h"

using namespace std;

static int lastX, lastY;
static int clickX, clickY;
static bool isLDown, isMDown, isRDown;
static bool isShiftDown, isCtrlDown, isAltDown;
static uint lastSelected;

void handleKeyboardInput1(void)
{
	const float dist = 5.0f;

	isShiftDown = (glfwGetKey(GLFW_KEY_LSHIFT) == GLFW_PRESS) ?
		true : false;
	isCtrlDown = (glfwGetKey(GLFW_KEY_LCTRL) == GLFW_PRESS) ?
		true : false;
	isAltDown = (glfwGetKey(GLFW_KEY_LALT) == GLFW_PRESS) ?
		true : false;

	if (glfwGetKey('A')) {
		if (isShiftDown)
			gl::alphaVal -= 0.01;
		else
			gl::alphaVal += 0.01;
		cout << gl::alphaVal << endl;
	}

	if (glfwGetKey('Y')) {
		if (isShiftDown)
			drawable.setTime(drawable.getTime()-0.033333);
		else
			drawable.setTime(drawable.getTime()+0.033333);
	}

	if (glfwGetKey('U')) {
		if (isShiftDown)
			drawable.setOvrTime(drawable.getOvrTime()-0.033333);
		else
			drawable.setOvrTime(drawable.getOvrTime()+0.033333);
	}

	if (glfwGetKey('W')) {
		if (isShiftDown)
			cam->dolly(dist);
//			cam->moveInOut(dist);
		else
			cam->zoom(dist);
//			cam->setDistance(cam->getDistance()-dist);
	}

	if (glfwGetKey('S')) {
		if (isShiftDown)
			cam->dolly(-dist);
//			cam->moveInOut(-dist);
		else
			cam->zoom(-dist);
//			cam->setDistance(cam->getDistance()+dist);
	}

	if (glfwGetKey('M')) {
		if (isShiftDown)
			timeCycle.setMinute(timeCycle.getMinute()-1);
		else
			timeCycle.setMinute(timeCycle.getMinute()+1);
	}
	if (glfwGetKey('H')) {
		if (isShiftDown)
			timeCycle.setHour(timeCycle.getHour()-1);
		else
			timeCycle.setHour(timeCycle.getHour()+1);
	}

	int u, r, d, l;
	u = glfwGetKey(GLFW_KEY_UP);
	r = glfwGetKey(GLFW_KEY_RIGHT);
	d = glfwGetKey(GLFW_KEY_DOWN);
	l = glfwGetKey(GLFW_KEY_LEFT);
	if (u || r || d || l) {
		if (isShiftDown)
			player->setStateRunning();
		else
			player->setStateWalking();
/*
		quat dir =
			quat(0.0, 1.0, 0.0)*u +
			quat(1.0, 0.0, 0.0)*r +
			quat(0.0, -1.0, 0.0)*d +
			quat(-1.0, 0.0, 0.0)*l;
*/
		quat dir =
			quat(1.0, 0.0, 0.0)*u +
			quat(0.0, -1.0, 0.0)*r +
			quat(-1.0, 0.0, 0.0)*d +
			quat(0.0, 1.0, 0.0)*l;
//		float a = cam->getYaw()/2.0;
		float a = cam->getHeading()/2.0;
		quat q(cos(a), 0, 0, -sin(a));
//		dir = q.getConjugate() * dir * q;
		dir = q.conjugate() * dir * q;
		float n;
		if ((n = dir.norm()) != 0.0)
			player->frm.setForward(dir/n);
	} else {
		player->setStateStop();
	}
}

void handleKeyboardInput2(void)
{
}

void handleKeyboardInput(void)
{
	if(consoleVisible)
		handleKeyboardInput2();
	else
		handleKeyboardInput1();
}


void keypress1(int key, int state)
{
	if (state != GLFW_PRESS)
		return;

	switch (key) {
	case '`':
		consoleVisible = !consoleVisible;
		break;
	case 'Q':
	case GLFW_KEY_ESC:
		exitprog();
		break;
/*
	case 'M':
		int x, y;
		glfwGetMousePos(&x, &y);
		cout << x << " " << y << endl;
		glfwSetMousePos(x+30, y+30);
		break;
*/
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
			player->drawable->printFrames(0, 0);
		break;
	case 'I':
		if (lastSelected == 0)
			break;
		world->getInstance(lastSelected)->printInfo();
		break;
	case 'X':
		cam->doTarget = !cam->doTarget;
		break;
	case 'F':
		physStep(0);
		break;
	default:
		break;
	}
}

void keypress2(int key, int state)
{
	if (state != GLFW_PRESS)
		return;

	switch (key) {
	case '`':
		consoleVisible = !consoleVisible;
		break;
	}
}

void keypress(int key, int state)
{
	if(consoleVisible)
		keypress2(key, state);
	else
		keypress1(key, state);
}

void mouseButton(int button, int state)
{
	int x, y;
	glfwGetMousePos(&x, &y);
	if (state == GLFW_PRESS) {
		lastX = clickX = x;
		lastY = clickY = y;
		if (button == GLFW_MOUSE_BUTTON_LEFT)
			isLDown = true;
		if (button == GLFW_MOUSE_BUTTON_MIDDLE)
			isMDown = true;
		if (button == GLFW_MOUSE_BUTTON_RIGHT)
			isRDown = true;
	} else if (state == GLFW_RELEASE) {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			isLDown = false;
			if (clickX - x == 0 && clickY - y == 0) {
				cout << x << " " << y << endl;
				// read clicked object's index
				union {
					char bytes[4];
					int integer;
				} stencil;
				// fewer passes would probably be enough
				gl::stencilShift = 0;
				renderer->renderScene();
				glReadPixels(x, gl::height - y - 1, 1, 1,
				             GL_STENCIL_INDEX,
				             GL_UNSIGNED_INT, stencil.bytes);
				gl::stencilShift = 8;
				renderer->renderScene();
				glReadPixels(x, gl::height - y - 1, 1, 1,
				             GL_STENCIL_INDEX,
				             GL_UNSIGNED_INT, stencil.bytes+1);
				gl::stencilShift = 16;
				renderer->renderScene();
				glReadPixels(x, gl::height - y - 1, 1, 1,
				             GL_STENCIL_INDEX,
				             GL_UNSIGNED_INT, stencil.bytes+2);
				gl::stencilShift = 24;
				renderer->renderScene();
				glReadPixels(x, gl::height - y - 1, 1, 1,
				             GL_STENCIL_INDEX,
				             GL_UNSIGNED_INT, stencil.bytes+3);
				if(stencil.integer >= 0)
					lastSelected = stencil.integer;
				else
					enexList->enter(-stencil.integer);
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
	static int xoff = 0, yoff = 0;
	static bool wrappedLast = false;

	dx = float(lastX - x) / float(gl::width);
	dy = float(lastY - y) / float(gl::height);
	/* Wrap the mouse if it goes over the window border.
	 * Unfortunately, after glfwSetMousePos is done, there can be old
	 * events with an old mouse position,
	 * hence the check if the pointer was wrapped the last time. */
	if ((isLDown || isMDown || isRDown) &&
	    (x < 0 || y < 0 || x >= gl::width || y >= gl::height)) {
		if (wrappedLast) {
			dx = float(lastX-xoff - x) / float(gl::width);
			dy = float(lastY-yoff - y) / float(gl::height);
		}
		xoff = yoff = 0;
		while (x+xoff >= gl::width) xoff -= gl::width;
		while (y+yoff >= gl::height) yoff -= gl::height;
		while (x+xoff < 0) xoff += gl::width;
		while (y+yoff < 0) yoff += gl::height;
		glfwSetMousePos(x+xoff, y+yoff);
		wrappedLast = true;
	} else {
		wrappedLast = false;
		xoff = yoff = 0;
	}
	lastX = x+xoff;
	lastY = y+yoff;
	if (isLDown) {
		if (isShiftDown) {
			cam->turn(dx*2.0f, dy*2.0f);
//			cam->turnLR(dx*2.0f);
//			cam->turnUD(-dy*2.0f);
		} else {
			cam->orbit(dx*2.0f, -dy*2.0f);
//			cam->setYaw(cam->getYaw()+dx*2.0f);
//			cam->setPitch(cam->getPitch()-dy*2.0f);
		}
	}
	if (isMDown) {
/*
		if (isShiftDown) {
		} else {
			cam->panLR(-dx*8.0f);
			cam->panUD(-dy*8.0f);
		}
*/
	}
	if (isRDown) {
/*
		if (isShiftDown) {
		} else {
			cam->setDistance(cam->getDistance()+dx*12.0f);
		}
*/
	}
}


void mouseWheel(int pos)
{
	const float dist = 2.0f;
	static int lastPos = 0;

	int diff = pos - lastPos;
	lastPos = pos;

/*
	if (isShiftDown)
		cam->moveInOut(dist*diff);
	else
		cam->setDistance(cam->getDistance()-dist*diff);
*/
}

void handleJoystickInput(int joy)
{
	int nAxes = glfwGetJoystickParam(GLFW_JOYSTICK_1, GLFW_AXES);
	int nButtons = glfwGetJoystickParam(GLFW_JOYSTICK_1, GLFW_BUTTONS);
	static float *axes = 0;
	static uchar *buttons = 0;
	if (nAxes < 3 || nButtons < 6)
		return;
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
/*
	cam->moveInOut(-axes[2]*accel);
	cam->turnLR(-axes[0]/10.0f);
	cam->turnUD(axes[1]/10.0f);
*/
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
