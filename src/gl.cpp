#include "gta.h"

#include <typeinfo>

#include "world.h"
#include "directory.h"
#include "math.h"
#include "camera.h"
#include "pipeline.h"
#include "gl.h"
#include "primitives.h"

#include <GL/glut.h>

using namespace std;

static int ox, oy, mx, my;
static int isLDown, isMDown, isRDown;
static int isShiftDown, isCtrlDown, isAltDown;

namespace gl {

int width;
int height;
Pipeline simplePipe, lambertPipe, gtaPipe;
Pipeline *currentPipe;

GLuint whiteTex;
bool drawWire;
bool drawTransparent;

glm::mat4 modelMat;
glm::mat4 viewMat;
glm::mat4 projMat;

GLuint axes_vbo;
GLuint cube_vbo;

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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// 3d scene
	glEnable(GL_DEPTH_TEST);
	cam.look();

	// set up modelview mat
	modelMat = glm::mat4(1.0f);
	glm::mat4 modelView = viewMat * modelMat;
	glm::mat3 normal = glm::inverseTranspose(glm::mat3(modelView));

	glm::vec4 lightPos = glm::vec4(5.0f, 5.0f, 3.0f, 1.0f);
	lightPos = viewMat * lightPos;
	glm::vec3 lightCol = glm::vec3(0.5f, 0.5f, 0.5f);

	glm::vec3 amb = glm::vec3(0.49f, 0.47f, 0.33f);

	simplePipe.use();
	glBindTexture(GL_TEXTURE_2D, 0);
	glUniformMatrix4fv(gl::u_Projection, 1, GL_FALSE,
	                   glm::value_ptr(gl::projMat));
	glUniformMatrix4fv(gl::u_ModelView, 1, GL_FALSE,
	                   glm::value_ptr(modelView));
	glUniformMatrix3fv(gl::u_NormalMat, 1, GL_FALSE,
	                   glm::value_ptr(normal));
	glUniform4fv(gl::u_LightPos, 1, glm::value_ptr(lightPos));
	glUniform3fv(gl::u_LightCol, 1, glm::value_ptr(lightCol));
	glUniform3fv(gl::u_AmbientLight, 1, glm::value_ptr(amb));
	glUniform1i(u_Texture, 0);
	glBindTexture(GL_TEXTURE_2D, gl::whiteTex);

	drawAxes(glm::value_ptr(modelMat));

	gtaPipe.use();
	glUniformMatrix4fv(gl::u_Projection, 1, GL_FALSE,
	                   glm::value_ptr(gl::projMat));
	glUniformMatrix4fv(gl::u_ModelView, 1, GL_FALSE,
	                   glm::value_ptr(modelView));
	glUniformMatrix3fv(gl::u_NormalMat, 1, GL_FALSE,
	                   glm::value_ptr(normal));
	glUniform4fv(gl::u_LightPos, 1, glm::value_ptr(lightPos));
	glUniform3fv(gl::u_LightCol, 1, glm::value_ptr(lightCol));
	glUniform3fv(gl::u_AmbientLight, 1, glm::value_ptr(amb));
	glUniform1i(u_Texture, 0);

	drawWire = false;
//	drawable.draw(false);
//	drawable.draw(true);

//	drawWire = true;
//	drawable.draw(false);
//	drawable.draw(true);

	world.drawIslands();

	// 2d overlay
	glDisable(GL_DEPTH_TEST);
	simplePipe.use();
	viewMat = glm::mat4(1.0f);
	modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(100, 100, 0));
	modelView = viewMat * modelMat;
	projMat = glm::ortho(0, width, 0, height, -1, 1);
	glUniformMatrix4fv(gl::u_Projection, 1, GL_FALSE,
	                   glm::value_ptr(gl::projMat));
	glUniformMatrix4fv(gl::u_ModelView, 1, GL_FALSE,
	                   glm::value_ptr(modelView));

	glBindTexture(GL_TEXTURE_2D, gl::whiteTex);
/*
	glVertexAttrib4f(in_Color, 1.0f, 1.0f, 1.0f, 1.0f);
	glRasterPos2i(0,0);
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'a');
*/

	glutSwapBuffers();
}

void reshape(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, width, height);
}

void keypress(uchar key, int x, int y)
{
	switch (key) {
	case 'f':
		drawable.nextFrame();
		break;
	case 'W':
		cam.moveInOut(10);
		break;
	case 'w':
		cam.changeDistance(-10.0f);
		break;
	case 'S':
		cam.moveInOut(-10);
		break;
	case 's':
		cam.changeDistance(10.0f);
		break;
	case 'i':
		world.setInterior(world.getInterior()+1);
		cout << "Interior: " << world.getInterior() << endl;
		break;
	case 'I':
		world.setInterior(world.getInterior()-1);
		cout << "Interior: " << world.getInterior() << endl;
		break;
	case 'c':
		cout << "enter new camara coords: " << endl;
		float x, y, z;
		cin >> x >> y >> z;
		cam.setTarget(quat(x, y, z));
		break;
	case 'm':
		world.setMinute(world.getMinute()+1);
		cout << world.getHour() << ":" << world.getMinute() << endl;
		break;
	case 'M':
		world.setMinute(world.getMinute()-1);
		cout << world.getHour() << ":" << world.getMinute() << endl;
		break;
	case 'h':
		world.setHour(world.getHour()+1);
		cout << world.getHour() << ":" << world.getMinute() << endl;
		break;
	case 'H':
		world.setHour(world.getHour()-1);
		cout << world.getHour() << ":" << world.getMinute() << endl;
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
			mx = x;
			my = y;
			isLDown = 1;
		}
		if (button == GLUT_MIDDLE_BUTTON) {
			mx = x;
			my = y;
			isMDown = 1;
		}
		if (button == GLUT_RIGHT_BUTTON) {
			mx = x;
			my = y;
			isRDown = 1;
		}
	} else if (state == GLUT_UP) {
		if (button == GLUT_LEFT_BUTTON)
			isLDown = 0;
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
	glEnable(GL_DEPTH_TEST);
//	glDepthFunc(GL_LESS);
	glDepthFunc(GL_LEQUAL);
	
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
	gtaPipe.load("shader/gtaPipe.vert", "shader/simple.frag");

	cam.setPitch(PI/8.0f-PI/2.0f);
	cam.setDistance(20.0f);
	cam.setAspectRatio((GLfloat) width / height);
	cam.setTarget(quat(335.5654907, -159.0345306, 17.85120964));
//	cam.setTarget(quat(1664.125, -1560.851563, 23.3515625));


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

/*
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
	string fileName = "anim/ped.ifp";
	correctFileName(fileName);
	fileName = gamePath + PSEP_S + fileName;
	ifstream f(fileName.c_str(), ios::binary);
	anpk.read(f);
	f.close();

	for (uint i = 0; i < anpk.animList.size(); i++) {
		stringToLower(anpk.animList[i].name);
		if (anpk.animList[i].name == "walk_player") {
			drawable.attachAnim(anpk.animList[i]);
			break;
		}
	}
*/
}

void start(int *argc, char *argv[])
{
	width = 640;
	height = 480;

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
