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

void cleanUp(void)
{
	delete objectList;
	delete world;
	delete renderer;
	delete cam;

	delete simplePipe;
	delete gtaPipe;

	drawable.unload();
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
	glfwSetKeyCallback(keypress);
	glfwSetWindowSizeCallback(resize);
//	glfwEnable(GLFW_KEY_REPEAT);

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

		string fileName = getPath("anim/ped.ifp");
		ifstream f(fileName.c_str(), ios::binary);
		anpk.read(f);
		f.close();

		if (*argc >= 5)
			for (uint i = 0; i < anpk.animList.size(); i++)
				if (anpk.animList[i].name == argv[4]) {
					drawable.attachAnim(&anpk.animList[i]);
					break;
				}
	}

	initDone = true;

/*
	int ret = glfwGetJoystickParam(GLFW_JOYSTICK_1, GLFW_PRESENT);
	cout << "joystick: " << ret << endl;
*/

	while (running) {
		handleKeyboardInput();
		handleJoystickInput(GLFW_JOYSTICK_1);

		renderer->renderScene();

		glfwSwapBuffers();
	}

	cleanUp();

	glfwTerminate();
//	cout << "return from opengl thread\n";
	return NULL;
}

}
