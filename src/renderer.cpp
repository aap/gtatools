#include <pthread.h>

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "world.h"
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
using namespace std;

#include <ctime>

Renderer *renderer;

Drawable drawable;

volatile bool threadFinished;

clock_t oldtime;

static void *buildListThread(void *)
{
	world->buildRenderList();
	oldtime = clock();
	threadFinished = true;
	return 0;
}

void Renderer::renderOpaque(void)
{
	THREADCHECK();
	gl::drawTransparent = false;
	for (size_t i = 0; i < opaqueRenderList.size(); i++) {
		Instance *ip = opaqueRenderList[i].inst;
		WorldObject *op =
			static_cast<WorldObject*>(objectList->get(ip->id));
		int ai = opaqueRenderList[i].atomic;

		glm::mat4 mvSave = gl::state.modelView;
		glm::mat3 nrmSave = gl::state.normalMat;

		ip->transform();

		glStencilFunc(GL_ALWAYS,
		              (ip->index>>gl::stencilShift)&0xFF,-1);

		globalAlpha = ip->getFading();
		gl::wasTransparent = false;
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

		if (gl::wasTransparent) {
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
 	gl::drawTransparent = true;
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
	gl::drawTransparent = true;
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
	        GL_STENCIL_BUFFER_BIT);

	timeCycle.calcCurrent();
	Weather *w = timeCycle.getCurrentWeatherData();

	// 3d scene
	cam->setNearFar(0.1f, w->farClp);
	cam->look();	// this sets the projection and modelView matrices

	gl::state.calculateNormalMat();

	gl::state.lightCol = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec4 lightDir = glm::vec4(1.0f, 1.0f, -1.0f, 0.0f);
	lightDir = gl::state.modelView * glm::normalize(lightDir);
	gl::state.lightDir = glm::vec3(lightDir);

	glm::vec3 amb;
	if (doTrails)
		amb = glm::vec3(w->ambBl.x, w->ambBl.y, w->ambBl.z);
	else
		amb = glm::vec3(w->amb.x, w->amb.y, w->amb.z);

	gl::simplePipe->use();

	gl::state.ambientLight = amb;
	gl::state.texture = 0;
	gl::state.updateAll();
	glBindTexture(GL_TEXTURE_2D, gl::whiteTex);

	glDisable(GL_DEPTH_TEST);
	sky.draw();
	glEnable(GL_DEPTH_TEST);

	if (cam->doTarget) {
		gl::drawAxes(glm::value_ptr(player->frm.mat));
		cam->drawTarget();
	}

	gl::gtaPipe->use();

	gl::state.fogColor.x = w->skyBot.x;
	gl::state.fogColor.y = w->skyBot.y;
	gl::state.fogColor.z = w->skyBot.z;
	gl::state.fogDensity = 0.0f;	// 0.0025f for SA perhaps ?
	if (!doFog)
		gl::state.fogDensity = -1.0f;
	gl::state.fogStart = w->fogSt;
	gl::state.fogEnd = w->farClp;
	gl::state.updateAll();

	gl::drawWire = false;

	// the test object
	if (drawable.hasModel() &&
	    (drawable.hasTextures() || !doTextures)) {
		gl::drawTransparent = false;
		drawable.draw();
		gl::drawTransparent = true;
		drawable.draw();
	}
	player->draw();

	// the world

	opaqueRenderList.clear();
	transp1RenderList.clear();
	transp2RenderList.clear();

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
	water.draw();
	if (doBFC)
		glEnable(GL_CULL_FACE);
	renderTransp2();
	globalAlpha = 1.0f;
	glDisable(GL_CULL_FACE);
}

Renderer::Renderer(void)
{
	doZones = false;
	doTextures = true;
	doFog = true;
	doVertexColors = true;
	doCol = false;
	doTrails = false;
	doBFC = false;
	lodMult = 1.0f;
	globalAlpha = 1.0f;
}
