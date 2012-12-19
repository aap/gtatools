#include <pthread.h>

#include <iostream>
#include <string>

#include <GL/glew.h>
#include <GL/glfw.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gta.h"
#include "math.h"
#include "pipeline.h"
#include "renderer.h"
#include "drawable.h"
#include "camera.h"
#include "objects.h"
#include "world.h"
#include "timecycle.h"
#include "animation.h"
#include "lua.h"
#include "gl.h"

#include "jobqueue.h"
#include "directory.h"

using namespace std;

namespace gl {

static GLuint axes_vbo;

Pipeline *simplePipe, *lambertPipe, *gtaPipe;

int stencilShift;
int width;
int height;
Pipeline *currentPipe;

GLuint whiteTex;
bool drawWire;
bool drawTransparent;
bool wasTransparent;

AnimPackage anpk;

void resize(int w, int h)
{
	width = w;
	height = h;
	if (height == 0)
		height = 1;
	cam->setAspectRatio(GLfloat(width)/GLfloat(height));
	glViewport(0, 0, width, height);
}


int initGl(void)
{
	if (glewInit() != GLEW_OK) {
		cerr << "couldn't init glew\n";
		return 1;
	}

	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClearDepth(1.0);
	glClearStencil(0);
	glEnable(GL_DEPTH_TEST);
//	glDepthFunc(GL_LESS);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	int white = 0xFFFFFFFF;
	glGenTextures(1, &whiteTex);
	glBindTexture(GL_TEXTURE_2D, whiteTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, 4,
		     1, 1, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, &white);

	simplePipe = new Pipeline("shader/simple.vert", "shader/simple.frag");
//	lambertPipe = new Pipeline("shader/lambert.vert", "shader/simple.frag");
	gtaPipe = new Pipeline("shader/gtaPipe.vert", "shader/gtaPipe.frag");

	GLfloat axes[] = {
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,

		1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f
	};
	glGenBuffers(1, &axes_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, axes_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return 0;
}

void *opengl(void *args)
{
	void **a = (void**)args;
	int *argc = (int *) (a[0]);
	char **argv= (char **) (a[1]);

	oglThread = pthread_self();

	width = 644;
	height = 340;

	if (!glfwInit()) {
		cerr << "Error: could not initialize GLFW\n";
		exitprog();
		return NULL;
	}
	if (!glfwOpenWindow(width, height,
	                    0, 0, 0, 0,
	                    24, 8,
	                    GLFW_WINDOW)) {
		cerr << "Error: could not create GLFW window\n";
		glfwTerminate();
		exitprog();
		return NULL;
	}

	if (initGl()) {
		exitprog();
		return NULL;
	}

	if (initGame()) {
		exitprog();
		return NULL;
	}

	glfwSetMouseButtonCallback(mouseButton);
	glfwSetMousePosCallback(mouseMotion);
	glfwSetMouseWheelCallback(mouseWheel);
	glfwSetKeyCallback(keypress);
	glfwSetWindowSizeCallback(resize);

	string fileName = getPath("anim/ped.ifp");
	ifstream f(fileName.c_str(), ios::binary);
	anpk.read(f);
	f.close();

	// Load the test object
	if (*argc >= 4) {
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

		if (*argc >= 5)
			for (size_t i = 0; i < anpk.animList.size(); i++)
				if (anpk.animList[i].name == argv[4]) {
					drawable.attachAnim(&anpk.animList[i]);
					break;
				}
	}

	luaInit();

/*
	int ret = glfwGetJoystickParam(GLFW_JOYSTICK_1, GLFW_PRESENT);
	cout << "joystick: " << ret << endl;
*/

	int frm = 0;
	clock_t startTime = clock();
	clock_t lastTime, time;
	lastTime = clock();
	while (running) {
		time = clock();
		handleKeyboardInput();
		handleJoystickInput(GLFW_JOYSTICK_1);
		updateGame((time - lastTime)*1.0/CLOCKS_PER_SEC);

		renderer->renderScene();

		glfwSwapBuffers();
		lastTime = time;
		frm++;
		if ((time - startTime) >= CLOCKS_PER_SEC) {
//			cout << frm << " fps\n";
			frm = 0;
			startTime = time;
		}
	}

	cleanUp();

	glfwTerminate();
//	cout << "return from opengl thread\n";
	return NULL;
}

}

void RefFrame::update(void)
{
	right = forward.wedge(up);
	up = right.wedge(forward);

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
	quat axis = forward.wedge(quat(0.0, 1.0, 0.0));
	if (axis.z == 0) {
		if (forward.y == 1)
			a = 0;
		else
			a = PI/2;
		axis = quat(0.0, 0.0, 1.0);
	}
	axis.normalize();
	quat q(cos(a), sin(a)*axis.x, sin(a)*axis.y, sin(a)*axis.z);
	diff = q.getConjugate() * diff * q;
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
	forward = q.getConjugate() * forward * q;
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
