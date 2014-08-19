#include <pthread.h>

#include <iostream>
#include <string>

#include <GL/glew.h>
#include <GL/glfw.h>

#include <glm/glm.hpp>

#include "gta.h"
#include "gl.h"
#include "primitives.h"
#include "math.h"
#include "jobqueue.h"
#include "lua.h"
#include "renderer.h"
#include "camera.h"
#include "console.h"

using namespace std;

namespace gl {
	int stencilShift;
	int width;
	int height;

	float alphaVal;
}

void resize(int w, int h)
{
	gl::width = w;
	gl::height = h;
	if (gl::height == 0)
		gl::height = 1;
//	cam->setAspectRatio(GLfloat(gl::width)/GLfloat(gl::height));
	cam->setAspectRatio(GLfloat(4.0/3.0));
	console->setDimensions(gl::width, gl::height);
	glViewport(0, 0, gl::width, gl::height);
}

void *filereader(void *)
{
	while (running) {
		while(normalJobs.processJob());
		if (!running)
			break;
		normalJobs.waitForJobs();
	}
	return NULL;
}

void *lua(void *)
{
	luaInterpreter();
	return NULL;
}


int main(int argc, char *argv[])
{
	progname = argv[0];
	if (argc < 2) {
		cerr << "usage: " << argv[0] << " game_path\n";
		return 1;
	}
	gamePath = argv[1];

	oglThread = pthread_self();

//	gl::width = 644;
//	gl::height = 340;
	gl::width = 640;
	gl::height = 512;

	gl::alphaVal = 1.0;

	if (!glfwInit()) {
		cerr << "Error: could not initialize GLFW\n";
		exitprog();
		return 1;
	}
	if (!glfwOpenWindow(gl::width, gl::height,
	                    0, 0, 0, 0,
	                    32, 8,
	                    GLFW_WINDOW)) {
		cerr << "Error: could not create GLFW window\n";
		glfwTerminate();
		exitprog();
		return 1;
	}

	GLint depth;
	glGetIntegerv(GL_DEPTH_BITS, &depth);
	cout << "z-depth: " << depth << endl;

	if (initGame()) {
		exitprog();
		return 1;
	}

/*
	// Load the test object
	if (argc >= 4) {
		string dff = argv[2];
		string txd = argv[3];
		if (txd == "search") {
			cout << "searching " << dff << endl;
			for (int i = 0; i < objectList->getObjectCount(); i++) {
				Model *mdl;
				if (!(mdl = objectList->get(i)))
					continue;
				if (mdl->type != OBJS && mdl->type != TOBJ &&
				    mdl->type != ANIM && mdl->type != PEDS &&
				    mdl->type != CARS && mdl->type != WEAP)
					continue;
				if (mdl->modelName == dff) {
					txd = mdl->textureName;
					cout << "found texdict: "
					     << txd << endl;
				}
			}
		}

		dff += ".dff";
		txd += ".txd";
		if (drawable.loadSynch(dff, txd) == -1)
			exit(1);

		if (argc >= 5)
			for (size_t i = 0; i < anpk.animList.size(); i++)
				if (anpk.animList[i].name == argv[4]) {
					drawable.attachAnim(&anpk.animList[i]);
					break;
				}
	}
*/

	glfwSetMouseButtonCallback(mouseButton);
	glfwSetMousePosCallback(mouseMotion);
	glfwSetMouseWheelCallback(mouseWheel);
	glfwSetKeyCallback(keypress);
	glfwSetWindowSizeCallback(resize);

	running = true;

	pthread_t thread1, thread2;

	pthread_create(&thread1, NULL, lua, NULL);
	pthread_create(&thread2, NULL, filereader, NULL);

/*
	int ret = glfwGetJoystickParam(GLFW_JOYSTICK_1, GLFW_PRESENT);
	cout << "joystick: " << ret << endl;
*/

	int frm = 0;
	double startTime = glfwGetTime();
	double lastTime, time;
	lastTime = glfwGetTime();
	while (running) {
		time = glfwGetTime();
		handleKeyboardInput();
		handleJoystickInput(GLFW_JOYSTICK_1);
		updateGame((time - lastTime));

		renderer->renderScene();

		glfwSwapBuffers();
		lastTime = time;
		frm++;
		if ((time - startTime) >= 1.0) {
//			cout << dec << frm << " fps\n";
			frm = 0;
			startTime = time;
		}
	}

	cleanUp();

	glfwTerminate();
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	cout << endl;

	return 0;
}

void RefFrame::update(void)
{
//	right = forward.wedge(up);
//	up = right.wedge(forward);
	right = forward^up;
	up = right^forward;

	glm::vec4 x, y, z, w;
	x.x = right.x; x.y = right.y; x.z = right.z; x.w = 0;
	y.x = forward.x; y.y = forward.y; y.z = forward.z; y.w = 0;
	z.x = up.x; z.y = up.y; z.z = up.z; z.w = 0;
	w.x = position.x; w.y = position.y; w.z = position.z; w.w = 1;
	mat = glm::mat4(x, y, z, w);
}

void RefFrame::move(quat p1, quat p2)
{
	quat diff = p2 - p1;
	float a = acos(forward.y)/2;
//	quat axis = forward.wedge(quat(0.0, 1.0, 0.0));
	quat axis = forward^quat(0.0, 1.0, 0.0);
	if (axis.z == 0) {
		if (forward.y == 1)
			a = 0;
		else
			a = PI/2;
		axis = quat(0.0, 0.0, 1.0);
	}
	axis.normalize();
	quat q(cos(a), sin(a)*axis.x, sin(a)*axis.y, sin(a)*axis.z);
//	diff = q.getConjugate() * diff * q;
	diff = q.conjugate() * diff * q;
	position += diff;
	update();
}

void RefFrame::moveForward(float d)
{
	position += forward*d;
	update();
}

void RefFrame::rotate(float r)
{
	r /= 2;
	quat q(cos(r), sin(r)*up.x, sin(r)*up.y, sin(r)*up.z);
//	forward = q.getConjugate() * forward * q;
	forward = q.conjugate() * forward * q;
	update();
}

void RefFrame::setForward(quat f)
{
	forward = f;
	update();
}

void RefFrame::setHeading(float r)
{
	forward = quat(0, sin(r), cos(r), 0);
	update();
}

void RefFrame::drawSphere(float r)
{
	glm::mat4 mvSave = gl::state.modelView;
	glm::mat3 nrmSave = gl::state.normalMat;

	gl::state.modelView *= mat;

	gl::state.calculateNormalMat();
	gl::state.updateMatrices();

	gl::drawSphere(r, 5, 5);

	gl::state.modelView = mvSave;
	gl::state.normalMat = nrmSave;
	gl::state.updateMatrices();
}

