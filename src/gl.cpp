#include <typeinfo>
#include <cstdio>

#include <pthread.h>

#include "gta.h"
#include "lua.h"

#include <GL/glut.h>

#include "world.h"
#include "directory.h"
#include "math.h"
#include "camera.h"
#include "pipeline.h"
#include "gl.h"
#include "primitives.h"
#include "water.h"
#include "timecycle.h"
#include "sky.h"
#include "renderer.h"

using namespace std;

namespace gl {

static int ox, oy, mx, my;
static int isLDown, isMDown, isRDown;
static int isShiftDown, isCtrlDown, isAltDown;
static uint lastSelected;
static string statusLine;
static GLuint axes_vbo;

Pipeline simplePipe, lambertPipe, gtaPipe;

int stencilShift;
int width;
int height;
Pipeline *currentPipe;

GLuint whiteTex;
bool drawWire;
bool drawTransparent;
bool wasTransparent;

void renderScene(void)
{
	if (!running) {
		cout << endl;
		exit(0);
	}

	renderer.renderScene();

	// 2d overlay
	glDisable(GL_DEPTH_TEST);
	simplePipe.use();
	state.projection = glm::ortho(0, width, 0, height, -1, 1);
	state.modelView= glm::translate(glm::mat4(1.0f), glm::vec3(100,100,0));
	state.updateMatrices();

//	glBindTexture(GL_TEXTURE_2D, whiteTex);
	glBindTexture(GL_TEXTURE_2D, 0);
	glVertexAttrib4f(in_Color, 0.0f, 0.0f, 0.0f, 0.0f);
	glRasterPos2i(0,0);

	for (uint i = 0; i < statusLine.size(); i++)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, statusLine[i]);

	glutSwapBuffers();
}

void reshape(int w, int h)
{
	width = w;
	height = h;
	if (height == 0)
		height = 1;
	cam.setAspectRatio((GLfloat) width / height);
	glViewport(0, 0, width, height);
}

void keypress(uchar key, int x, int y)
{
	char buf[20];
	WorldObject *op;
	quat campos;
	static float dist = 10.0f;

	switch (key) {
	case 'f':
		drawable.nextFrame();
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
		op = (WorldObject*)objectList.get(
			world.getInstance(lastSelected)->id);
		op->drawable.dumpClump(false);
		break;
	case 'D':
		op = (WorldObject*)objectList.get(
			world.getInstance(lastSelected)->id);
		op->drawable.dumpClump(true);
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
		world.getInstance(lastSelected)->setVisible(false);
		break;
	case 'V':
		if (lastSelected == 0)
			break;
		world.getInstance(lastSelected)->setVisible(true);
		break;
	case 'b':
		if (lastSelected == 0)
			break;
		op = (WorldObject*)objectList.get(
			world.getInstance(lastSelected)->id);
		op->BSvisible = true;
		break;
	case 'B':
		if (lastSelected == 0)
			break;
		op = (WorldObject*)objectList.get(
			world.getInstance(lastSelected)->id);
		op->BSvisible = false;
		break;
	case 'i':
		if (lastSelected == 0)
			break;
		world.getInstance(lastSelected)->printInfo();
		break;
	case 'q':
	case 27:
		running = false;
		break;
	default:
		break;
	}
}

void keypressSpecial(int key, int x, int y)
{
}

void mouseButton(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) {
		if (button == GLUT_LEFT_BUTTON) {
			ox = mx = x;
			oy = my = y;
			isLDown = 1;
		}
		if (button == GLUT_MIDDLE_BUTTON) {
			ox = mx = x;
			oy = my = y;
			isMDown = 1;
		}
		if (button == GLUT_RIGHT_BUTTON) {
			ox = mx = x;
			oy = my = y;
			isRDown = 1;
		}
	} else if (state == GLUT_UP) {
		if (button == GLUT_LEFT_BUTTON) {
			isLDown = 0;
			if (ox - mx == 0 && oy - my == 0) {
				cout << x << " " << y << endl;
				// read clicked object's index
				GLubyte stencil[4];
				// fewer passes would probably be enough
				stencilShift = 0;
				renderScene();
				glReadPixels(x, height - y - 1, 1, 1,
				             GL_STENCIL_INDEX,
				             GL_UNSIGNED_INT, stencil);
				stencilShift = 8;
				renderScene();
				glReadPixels(x, height - y - 1, 1, 1,
				             GL_STENCIL_INDEX,
				             GL_UNSIGNED_INT, stencil+1);
				stencilShift = 16;
				renderScene();
				glReadPixels(x, height - y - 1, 1, 1,
				             GL_STENCIL_INDEX,
				             GL_UNSIGNED_INT, stencil+2);
				stencilShift = 24;
				renderScene();
				glReadPixels(x, height - y - 1, 1, 1,
				             GL_STENCIL_INDEX,
				             GL_UNSIGNED_INT, stencil+3);
				lastSelected = *((uint *) stencil);
			}
		}
		if (button == GLUT_MIDDLE_BUTTON)
			isMDown = 0;
		if (button == GLUT_RIGHT_BUTTON)
			isRDown = 0;
	}

	int mod = glutGetModifiers();
	isShiftDown = (mod & GLUT_ACTIVE_SHIFT) ? 1 : 0;
	isCtrlDown = (mod & GLUT_ACTIVE_CTRL) ? 1 : 0;
	isAltDown = (mod & GLUT_ACTIVE_ALT) ? 1 : 0;
}

void mouseMotion(int x, int y)
{
	GLfloat dx, dy;

	ox = mx;
	oy = my;
	mx = x;
	my = y;

	dx = ((float)(ox - mx) / width);
	dy = ((float)(oy - my) / height);
	if (!isShiftDown) {
		if (isLDown) {
			cam.setYaw(cam.getYaw()+dx*2.0f);
			cam.setPitch(cam.getPitch()-dy*2.0f);
		}
		if (isMDown) {
			cam.panLR(-dx*8.0f);
			cam.panUD(-dy*8.0f);
		}
		if (isRDown) {
			cam.setDistance(cam.getDistance()+dx*12.0f);
		}
	} else if (isShiftDown) {
		if (isLDown) {
			cam.turnLR(dx*2.0f);
			cam.turnUD(-dy*2.0f);
		}
		if (isMDown) {
		}
		if (isRDown) {
		}
	}
}

void initGl(void)
{
	if (glewInit() != GLEW_OK) {
		cerr << "couldn't init glew\n";
		exit(1);
	}

	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClearDepth(1.0);
	glClearStencil(0);
	glEnable(GL_DEPTH_TEST);
//	glDepthFunc(GL_LESS);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	renderer.init();

	int white = 0xFFFFFFFF;
	glGenTextures(1, &whiteTex);
	glBindTexture(GL_TEXTURE_2D, whiteTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, 4,
		     1, 1, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, &white);

	simplePipe.load("shader/simple.vert", "shader/simple.frag");
	lambertPipe.load("shader/lambert.vert", "shader/simple.frag");
	gtaPipe.load("shader/gtaPipe.vert", "shader/gtaPipe.frag");

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
}

void *opengl(void *args);
void *loader(void *args);
void *lua(void *args);

void start(int *argc, char *argv[])
{
	width = 644;
	height = 340;
//	width = 640;
//	height = 480;

	glutInit(argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE |
	                    GLUT_ALPHA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(width, height);
	glutCreateWindow(progname);

	glutDisplayFunc(renderScene);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keypress);
	glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMotion);
	glutSpecialFunc(keypressSpecial);
	glutIdleFunc(renderScene);

	initGl();

	initGame();


if (*argc >= 4) {
	string txd = argv[2];
	string dff = argv[3];
	if (txd == "search") {
		cout << "searching " << dff << endl;
		for (int i = 0; i < objectList.getObjectCount(); i++) {
			Object *o;
			if (!(o = objectList.get(i)))
				continue;
			if (o->type != OBJS && o->type != TOBJ &&
			    o->type != ANIM && o->type != PEDS &&
			    o->type != CARS && o->type != WEAP)
				continue;
			Model *m = (Model*) o;
			if (m->modelName == dff) {
				txd = m->textureName;
				cout << "found texdict: " << txd << endl;
			}
		}
	}

	dff += ".dff";
	txd += ".txd";
	if (drawable.load(dff, txd) == -1)
		exit(1);

	AnimPackage anpk;
	string fileName = getPath("anim/ped.ifp");
	ifstream f(fileName.c_str(), ios::binary);
	anpk.read(f);
	f.close();

/*
	rw::AnimPackage anpk;
	ifstream f;
	if (directory.openFile(f, "sfw.ifp") == -1)
		cout << "whaa\n";
	anpk.read(f);
	f.close();
*/

	for (uint i = 0; i < anpk.animList.size(); i++) {
		stringToLower(anpk.animList[i].name);
//		if (anpk.animList[i].name == "sprasfw") {
		if (anpk.animList[i].name == "walk_player") {
			drawable.attachAnim(anpk.animList[i]);
			break;
		}
	}
}

	running = true;

	pthread_t thread1, thread2, thread3;
	pthread_create(&thread1, NULL, opengl, NULL);
	pthread_create(&thread2, NULL, loader, NULL);
	pthread_create(&thread3, NULL, lua, NULL);

	// this will never happen
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	pthread_join(thread3, NULL);
}

void *opengl(void *args)
{
	glutMainLoop();
	return NULL;
}

void *loader(void *args)
{
//	objMan.loaderLoop();
	return NULL;
}

void *lua(void *args)
{
	LuaInterpreter();
	return NULL;
}

}
