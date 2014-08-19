#include <pthread.h>

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "world.h"
#include "enex.h"
#include "drawable.h"
#include "objects.h"
#include "camera.h"
#include "timecycle.h"
#include "sky.h"
#include "water.h"
#include "gl.h"
#include "pipeline.h"
#include "primitives.h"
#include "renderer.h"
#include "jobqueue.h"
#include "phys.h"
#include "console.h"
using namespace std;

#include <ctime>

Renderer *renderer;
static GLuint axes_vbo;
clock_t oldtime;

Drawable drawable;

volatile bool threadFinished;

static void *buildListThread(void *)
{
	world->buildRenderList();
	oldtime = clock();
	threadFinished = true;
	return 0;
}

int Renderer::init(void)
{
	if (glewInit() != GLEW_OK) {
		cerr << "couldn't init glew\n";
		return 1;
	}

	freeze = false;

	simplePipe=new gl::Pipeline("shader/simple.vert", "shader/simple.frag");
	gtaPipe=new gl::Pipeline("shader/gtaPipe.vert", "shader/gtaPipe.frag");

	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

//	glClearDepth(1.0);
	glClearDepth(0.0);
	glDepthRange(1.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_GREATER);
//	glDepthFunc(GL_LESS);
//	glDepthFunc(GL_LEQUAL);

	glClearStencil(0);
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

	GLfloat vertices[] = {
		1.0, 0.0, 0.0,
		0.0, 0.0, 0.0,
		1.0, 1.0, 0.0,
		0.0, 0.0, 0.0,
		1.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
	};
	glGenBuffers(1, &rectvbo);
	glBindBuffer(GL_ARRAY_BUFFER, rectvbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
		&vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return 0;
}

int Renderer::reloadShaders(void)
{
	delete simplePipe;
	delete gtaPipe;
	simplePipe=new gl::Pipeline("shader/simple.vert", "shader/simple.frag");
	gtaPipe=new gl::Pipeline("shader/gtaPipe.vert", "shader/gtaPipe.frag");

}

void Renderer::renderOpaque(void)
{
	THREADCHECK();
	drawTransparent = false;
	for (size_t i = 0; i < opaqueRenderList.size(); i++) {
		Instance *ip = opaqueRenderList[i].inst;
		WorldObject *op =
			static_cast<WorldObject*>(objectList->get(ip->id));
		int ai = opaqueRenderList[i].atomic;

		glm::mat4 mvSave = gl::state.modelView;
		glm::mat3 nrmSave = gl::state.normalMat;

		ip->transform();

		glStencilFunc(GL_ALWAYS, (ip->index>>gl::stencilShift)&0xFF,-1);

		globalAlpha = ip->getFading();
		wasTransparent = false;
		if (doCol) {
			if (op->col)
				op->drawCol();
		} else {
			if (op->drawable == 0) {
				cout << "no drawable " << op->modelName << endl;
				goto next;
			}
			if (!op->canDraw()) {
				cout << "cant draw " << op->modelName << endl;
				goto next;
			}
			if (ai == -1)
				op->drawable->draw();
			else
				op->drawable->drawAtomic(ai);
		}

		op->isFreshlyLoaded = false;

		if (wasTransparent) {
			if (op->flags & 0x40)
				addTransp1Object(ip, ai);
			else
				addTransp2Object(ip, ai);
		}

		if (op->BSvisible)
			op->drawBoundingSphere();

next:
		gl::state.modelView = mvSave;
		gl::state.normalMat = nrmSave;
		gl::state.updateMatrices();

		ip->wasAdded = false;
	}
//	cout << dec << opaqueRenderList.size() << " insts drawn\n";
}

void Renderer::renderTransp1(void)
{
	THREADCHECK();
 	drawTransparent = true;
	glEnable(GL_ALPHA_TEST);
	for (size_t i = 0; i < transp1RenderList.size(); i++) {
		Instance *ip = transp1RenderList[i].inst;
		WorldObject *op =
			static_cast<WorldObject*>(objectList->get(ip->id));
		int ai = transp1RenderList[i].atomic;

		glm::mat4 mvSave = gl::state.modelView;
		glm::mat3 nrmSave = gl::state.normalMat;

		ip->transform();

		glStencilFunc(GL_ALWAYS,
		              (ip->index>>gl::stencilShift)&0xFF,-1);
		if (op->flags & 0x2000)
			glAlphaFunc(GL_GREATER, 0.5);
		else
			glAlphaFunc(GL_GREATER, 0.0);

		globalAlpha = ip->getFading();
		if (ai == -1)
			op->drawable->draw();
		else
			op->drawable->drawAtomic(ai);

		gl::state.modelView = mvSave;
		gl::state.normalMat = nrmSave;
		gl::state.updateMatrices();
	}
}

void Renderer::renderTransp2(void)
{
	THREADCHECK();
	drawTransparent = true;
	glEnable(GL_ALPHA_TEST);
	for (size_t i = 0; i < transp2RenderList.size(); i++) {
		Instance *ip = transp2RenderList[i].inst;
		WorldObject *op =
			static_cast<WorldObject*>(objectList->get(ip->id));
		int ai = transp2RenderList[i].atomic;

		glm::mat4 mvSave = gl::state.modelView;
		glm::mat3 nrmSave = gl::state.normalMat;

		ip->transform();

		glStencilFunc(GL_ALWAYS,
		              (ip->index>>gl::stencilShift)&0xFF,-1);
		if (op->flags & 0x2000)
			glAlphaFunc(GL_GREATER, 0.5);
		else
			glAlphaFunc(GL_GREATER, 0.0);


		globalAlpha = ip->getFading();
		if (ai == -1)
			op->drawable->draw();
		else
			op->drawable->drawAtomic(ai);

		gl::state.modelView = mvSave;
		gl::state.normalMat = nrmSave;
		gl::state.updateMatrices();
	}

	if (doZones)
//		world->drawZones();
		world->drawSectors();
}

void Renderer::addOpaqueObject(Instance *ip, int a)
{
	RenderListObject o;

	ip->wasAdded = true;
	o.inst = ip;
	o.atomic = a;
	WorldObject *op = static_cast<WorldObject*>(objectList->get(ip->id));

	if (op->drawable == 0)
		cout << "no drawable adding " << op->modelName << endl;

	if (op->flags & 4)
		opaqueRenderList.push_back(o);
	else
		opaqueRenderList.push_front(o);
}

void Renderer::addTransp1Object(Instance *ip, int a)
{
	RenderListObject o;

	o.inst = ip;
	o.atomic = a;
	transp1RenderList.push_back(o);
}

void Renderer::addTransp2Object(Instance *ip, int a)
{
	RenderListObject o;

	o.inst = ip;
	o.atomic = a;
	transp2RenderList.push_back(o);
}

void Renderer::renderScene(void)
{
	THREADCHECK();

	if(doReloadShaders){
		reloadShaders();
		doReloadShaders = false;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
	        GL_STENCIL_BUFFER_BIT);

	timeCycle.calcCurrent();
	Weather *w = timeCycle.getCurrentWeatherData();

	// 3d scene
	cam->setNearFar(0.1f, game == GTASA ? 450.0f : w->farClp);
	cam->look();	// this sets the projection and modelView matrices

	gl::state.calculateNormalMat();

	gl::state.lightCol = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec4 lightDir = glm::vec4(1.0f, 1.0f, -1.0f, 0.0f);
	lightDir = gl::state.modelView * glm::normalize(lightDir);
	gl::state.lightDir = glm::vec3(lightDir);

	if (doTrails) {
		gl::state.col1[0] = w->rgba1.x;
		gl::state.col1[1] = w->rgba1.y;
		gl::state.col1[2] = w->rgba1.z;
		gl::state.col1[3] = w->rgba1.w;

		gl::state.col2[0] = w->rgba2.x;
		gl::state.col2[1] = w->rgba2.y;
		gl::state.col2[2] = w->rgba2.z;
		gl::state.col2[3] = w->rgba2.w;
	} else {
		gl::state.col1[0] = 0;
		gl::state.col1[1] = 0;
		gl::state.col1[2] = 0;
		gl::state.col1[3] = 0;

		gl::state.col2[0] = 1.0;
		gl::state.col2[1] = 1.0;
		gl::state.col2[2] = 1.0;
		gl::state.col2[3] = 0.5;
	}

	glm::vec3 amb;
	if (doTrails)
		amb = glm::vec3(w->ambBl.x, w->ambBl.y, w->ambBl.z);
	else
		amb = glm::vec3(w->amb.x, w->amb.y, w->amb.z);

	simplePipe->use();

	gl::state.ambientLight = amb;
	gl::state.texture = 0;
	gl::state.textureType = 0;
	gl::state.updateAll();
	glBindTexture(GL_TEXTURE_2D, whiteTex);

	glDisable(GL_DEPTH_TEST);
	sky.draw();
	glEnable(GL_DEPTH_TEST);

	if (cam->doTarget) {
		gl::drawAxes(glm::value_ptr(player->frm.mat));
		cam->drawTarget();
	}

	gtaPipe->use();

	gl::state.fogColor.x = w->skyBot.x;
	gl::state.fogColor.y = w->skyBot.y;
	gl::state.fogColor.z = w->skyBot.z;
//	gl::state.fogDensity = 0.0025f;	// 0.0025f for SA perhaps ?
	gl::state.fogDensity = gl::alphaVal;
	if (!doFog)
		gl::state.fogDensity = -1.0f;
	gl::state.fogStart = w->fogSt;
	gl::state.fogEnd = game == GTASA ? 450 : w->farClp;
	gl::state.updateAll();

	drawWire = false;

	// the test object
	if (drawable.hasModel() &&
	    (drawable.hasTextures() || !doTextures)) {
		drawTransparent = false;
		drawable.draw();
		drawTransparent = true;
		drawable.draw();
	}

	player->draw();

	coltest->drawSphere(1);

	// the world

	transp1RenderList.clear();
	transp2RenderList.clear();

	if(!freeze){
		opaqueRenderList.clear();

	//	world->cleanUp();

		// Start thread to build render list
		pthread_t thr;
		threadFinished = false;
		if (pthread_create(&thr, NULL, buildListThread, NULL) != 0)
			cout << "pthread failed\n";

		while (glJobs.processJob())
			if (threadFinished)
				break;

		pthread_join(thr, NULL);
	}

	// Everything is done now, draw!
	if (doBFC)
		glEnable(GL_CULL_FACE);
	renderOpaque();	// this constructs the transp1/2 render lists
	globalAlpha = 1.0f;
	glDepthMask(GL_FALSE);

	renderTransp1();
	globalAlpha = 1.0f;
	glDepthMask(GL_TRUE);
	glDisable(GL_CULL_FACE);
	water->draw();
	if (doBFC)
		glEnable(GL_CULL_FACE);
	renderTransp2();
	globalAlpha = 1.0f;
	glDisable(GL_CULL_FACE);

//	enexList->draw();

	// 2d stuf

	simplePipe->use();
	gl::state.projection = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
	gl::state.modelView = glm::mat4(1.0f);
	gl::state.updateAll();

	if(doTrails && game == GTA3){
		glDisable(GL_DEPTH_TEST);

		glVertexAttrib4f(gl::in_Color, w->tint.x*0.6f, w->tint.y*0.6f, w->tint.z*0.6f, w->tint.w*0.6f);
		glBindTexture(GL_TEXTURE_2D, whiteTex);
		glBindBuffer(GL_ARRAY_BUFFER, rectvbo);
		glEnableVertexAttribArray(gl::in_Vertex);
		glVertexAttribPointer(gl::in_Vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(gl::in_Vertex);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	gl::state.projection = glm::ortho(0.0f, (float)gl::width, 0.0f, (float)gl::height, -1.0f, 1.0f);
	gl::state.updateMatrices();

	if(consoleVisible)
		console->draw();
}

Renderer::Renderer(void)
: simplePipe(0), gtaPipe(0),
  doZones(false), doTextures(true), doFog(true), doVertexColors(true),
  doCol(false), doTrails(false), doBFC(false), lodMult(1.0f), globalAlpha(1.0f)
{
}

Renderer::~Renderer(void)
{
	delete simplePipe;
	delete gtaPipe;
	simplePipe = gtaPipe = 0;
}

