#include <typeinfo>
#include <cstdio>
#include <cstdio>

#include "gta.h"

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

using namespace std;

static int ox, oy, mx, my;
static int isLDown, isMDown, isRDown;
static int isShiftDown, isCtrlDown, isAltDown;

namespace gl {

int stencilShift;
int width;
int height;
Pipeline simplePipe, lambertPipe, gtaPipe;
Pipeline *currentPipe;

GLuint whiteTex;
bool drawWire;
bool drawTransparent;
bool wasTransparent;
uint lastSelected;
string statusLine;

glm::mat4 modelMat;
glm::mat4 viewMat;
glm::mat4 projMat;

GLuint axes_vbo;

void dumpmat(glm::mat3 &m)
{
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			cout << m[j][i] << " ";
		cout << endl;
	}
	cout << endl;
}

void dumpmat(glm::mat4 &m)
{
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++)
			cout << m[j][i] << " ";
		cout << endl;
	}
	cout << endl;
}

void renderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
	        GL_STENCIL_BUFFER_BIT);

	Weather *w = timeCycle.getCurrentWeatherData();

	// 3d scene
	timeCycle.calcCurrent(timeCycle.getHour(), timeCycle.getMinute());
	cam.setNearFar(0.1f, w->farClp);
	cam.look();

	// set up modelview mat
	modelMat = glm::mat4(1.0f);
	glm::mat4 modelView = viewMat * modelMat;
	glm::mat3 normal = glm::inverseTranspose(glm::mat3(modelView));

	// not used at the moment
	glm::vec4 lightPos = glm::vec4(5.0f, 5.0f, 3.0f, 1.0f);
	lightPos = viewMat * lightPos;
	glm::vec3 lightCol = glm::vec3(1.0f, 1.0f, 1.0f);

//	glm::vec3 amb = glm::vec3(0.49f, 0.47f, 0.33f);
	glm::vec3 amb = glm::vec3(w->amb.x, w->amb.y, w->amb.z);

	simplePipe.use();
	state.projection = projMat;
	state.modelView = modelView;
	state.normalMat = normal;
	state.lightPos = lightPos;
	state.lightCol = lightCol;
	state.ambientLight = amb;
	state.texture = 0;
	state.updateAll();
	glBindTexture(GL_TEXTURE_2D, gl::whiteTex);

	glDisable(GL_DEPTH_TEST);

	sky.draw();

	glEnable(GL_DEPTH_TEST);

	drawAxes(glm::value_ptr(modelMat));

	gtaPipe.use();
	state.fogColor.x = w->skyBot.x;
	state.fogColor.y = w->skyBot.y;
	state.fogColor.z = w->skyBot.z;
	state.fogColor.w = w->skyBot.w;
	state.fogDensity = 0.0f;	// 0.0025f for SA perhaps ?
	state.fogStart = w->fogSt;
	state.fogEnd = w->farClp;
	state.updateAll();

	drawWire = false;

/*
	drawTransparent = false;
	drawable.draw();
	drawTransparent = true;
	drawable.draw();
*/

	world.drawOpaque();

	glDepthMask(GL_FALSE);
	world.drawTransparent1();
	glDepthMask(GL_TRUE);

	water.draw();
	world.drawTransparent2();

	// 2d overlay
	glDisable(GL_DEPTH_TEST);
	simplePipe.use();
	viewMat = glm::mat4(1.0f);
	modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(100, 100, 0));
	modelView = viewMat * modelMat;
	projMat = glm::ortho(0, width, 0, height, -1, 1);

	state.projection = projMat;
	state.modelView = modelView;
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
		cam.changeDistance(-dist);
		break;
	case 'S':
		cam.moveInOut(-dist);
		break;
	case 's':
		cam.changeDistance(dist);
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
	case 'i':
		world.setInterior(world.getInterior()+1);
		cout << "Interior: " << world.getInterior() << endl;
		break;
	case 'I':
		world.setInterior(world.getInterior()-1);
		cout << "Interior: " << world.getInterior() << endl;
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
		world.getInstance(lastSelected)->setVisible(false);
		break;
	case 'V':
		world.getInstance(lastSelected)->setVisible(true);
		break;
	case 'p':
		campos = cam.getTarget();
		cout << campos << endl;
		break;
	case 'P':
		cout << "enter new camara coords: " << endl;
		float x, y, z;
		cin >> x >> y >> z;
		cam.setTarget(quat(x, y, z));
		break;
	case 'c':
		timeCycle.setColorStep(timeCycle.getColorStep()+1);
		cout << timeCycle.getColorStep() << endl;
		break;
	case 'C':
		timeCycle.setColorStep(timeCycle.getColorStep()-1);
		cout << timeCycle.getColorStep() << endl;
		break;
	case 'q':
	case 27:
		exit(0);
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
				world.getInstance(lastSelected)->printInfo();
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
			cam.changeYaw(dx*2.0f);
			cam.changePitch(-dy*2.0f);
		}
		if (isMDown) {
			cam.PanLR(-dx*8.0f);
			cam.PanUD(-dy*8.0f);
		}
		if (isRDown) {
			cam.changeDistance(dx*12.0f);
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

void init(char *model, char *texdict)
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

	cam.setPitch(PI/8.0f-PI/2.0f);
	cam.setDistance(0.0f);
//	cam.setDistance(20.0f);
//	cam.setDistance(5.0f);
	cam.setAspectRatio((GLfloat) width / height);
	cam.setTarget(quat(335.5654907, -159.0345306, 17.85120964));
//	cam.setTarget(quat(-1158.1, 412.282, 33.6813));	// dam
//	cam.setTarget(quat(1176.17, -1154.5, 87.2194));	// la records


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

	world.associateLods();

	// load water
	if (game == GTA3 || game == GTAVC) {
		string fileName = getPath("data/waterpro.dat");
		ifstream f(fileName.c_str(), ios::binary);
		if (f.fail()) {
			cerr << "couldn't open waterpro.dat\n";
		} else {
			water.loadWaterpro(f);
			f.close();
		}
	} else {	// must be GTASA
		string fileName = getPath("data/water.dat");
		ifstream f(fileName.c_str());
		if (f.fail()) {
			cerr << "couldn't open water.dat\n";
		} else {
			water.loadWater(f);
			f.close();
		}
	}

	// load timecycle
	string fileName = getPath("data/timecyc.dat");
	ifstream f(fileName.c_str());
	if (f.fail()) {
		cerr << "couldn't open timecyc.dat\n";
	} else {
		timeCycle.load(f);
		f.close();
	}

#if 0
	string txd = texdict;
	string dff = model;
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

	rw::AnimPackage anpk;
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
#endif
}

void start(int *argc, char *argv[])
{
	width = 644;
	height = 340;

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

	init(argv[2], argv[3]);

	glutMainLoop();
}

}
