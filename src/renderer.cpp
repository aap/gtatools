#include <pthread.h>

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "world.h"
#include "drawable.h"
#include "objects.h"
#include "objman.h"
#include "camera.h"
#include "timecycle.h"
#include "sky.h"
#include "water.h"
#include "gl.h"
#include "pipeline.h"
#include "primitives.h"
#include "renderer.h"
using namespace std;

Renderer renderer;

Drawable drawable;

volatile bool threadFinished;

static void *buildListThread(void *args)
{
	world.buildRenderList();
	threadFinished = true;
	return 0;
}

void Renderer::renderOpaque(void)
{
	gl::drawTransparent = false;
	for (uint i = 0; i < opaqueRenderList.size(); i++) {
		Instance *ip = opaqueRenderList[i].inst;
		WorldObject *op = (WorldObject*)objectList.get(ip->id);
		int ai = opaqueRenderList[i].atomic;

		glm::mat4 mvSave = gl::state.modelView;
		glm::mat3 nrmSave = gl::state.normalMat;

		ip->transform();

		glStencilFunc(GL_ALWAYS,(ip->index>>gl::stencilShift)&0xFF,-1);

		if (op->BSvisible)
			op->drawBoundingSphere();

		gl::wasTransparent = false;
		if (doCol) {
			if (op->col)
				op->drawCol();
		} else {
			if (ai == -1)
				op->drawable.draw();
			else
				op->drawable.drawAtomic(ai);
		}

		ip->wasAdded = false;

		if (gl::wasTransparent) {
			if (op->flags & 0x40)
				addTransp1Object(ip, ai);
			else
				addTransp2Object(ip, ai);
		}

		gl::state.modelView = mvSave;
		gl::state.normalMat = nrmSave;
		gl::state.updateMatrices();
	}
//	cout << dec << opaqueRenderList.size() << " insts drawn\n";
}

void Renderer::renderTransp1(void)
{
	gl::drawTransparent = true;
	for (uint i = 0; i < transp1RenderList.size(); i++) {
		Instance *ip = transp1RenderList[i].inst;
		WorldObject *op = (WorldObject*)objectList.get(ip->id);
		int ai = transp1RenderList[i].atomic;

		glm::mat4 mvSave = gl::state.modelView;
		glm::mat3 nrmSave = gl::state.normalMat;

		ip->transform();

		glStencilFunc(GL_ALWAYS,(ip->index>>gl::stencilShift)&0xFF,-1);

		if (ai == -1)
			op->drawable.draw();
		else
			op->drawable.drawAtomic(ai);

		gl::state.modelView = mvSave;
		gl::state.normalMat = nrmSave;
		gl::state.updateMatrices();
	}
}

void Renderer::renderTransp2(void)
{
	gl::drawTransparent = true;
	for (uint i = 0; i < transp2RenderList.size(); i++) {
		Instance *ip = transp2RenderList[i].inst;
		WorldObject *op = (WorldObject*)objectList.get(ip->id);
		int ai = transp2RenderList[i].atomic;

		glm::mat4 mvSave = gl::state.modelView;
		glm::mat3 nrmSave = gl::state.normalMat;

		ip->transform();

		glStencilFunc(GL_ALWAYS,(ip->index>>gl::stencilShift)&0xFF,-1);

		if (ai == -1)
			op->drawable.draw();
		else
			op->drawable.drawAtomic(ai);

		gl::state.modelView = mvSave;
		gl::state.normalMat = nrmSave;
		gl::state.updateMatrices();
	}

	if (doZones)
//		world.drawZones();
		world.drawSectors();
}

void Renderer::addOpaqueObject(Instance *ip, int a)
{
	RenderListObject o;

	ip->wasAdded = true;
	o.inst = ip;
	o.atomic = a;
	opaqueRenderList.push_back(o);
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
	        GL_STENCIL_BUFFER_BIT);

	Weather *w = timeCycle.getCurrentWeatherData();

	// 3d scene
	timeCycle.calcCurrent(timeCycle.getHour(), timeCycle.getMinute());
	cam.setNearFar(0.1f, w->farClp);
	cam.look();	// this sets the projection and modelView matrices

	gl::state.calculateNormalMat();

//	not used at the moment
//	glm::vec4 lightPos = glm::vec4(5.0f, 5.0f, 3.0f, 1.0f);
//	lightPos = gl::state.modelView * lightPos;
	gl::state.lightCol = glm::vec3(1.0f, 1.0f, 1.0f);

	glm::vec3 amb;
	if (doTrails)
		amb = glm::vec3(w->ambBl.x, w->ambBl.y, w->ambBl.z);
	else
		amb = glm::vec3(w->amb.x, w->amb.y, w->amb.z);

	gl::simplePipe.use();

	gl::state.ambientLight = amb;
	gl::state.texture = 0;
	gl::state.updateAll();
	glBindTexture(GL_TEXTURE_2D, gl::whiteTex);

	gl::drawAxes(glm::value_ptr(gl::state.modelView));

	glDisable(GL_DEPTH_TEST);
	sky.draw();
	glEnable(GL_DEPTH_TEST);


	gl::gtaPipe.use();

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

	gl::drawTransparent = false;
	drawable.draw();
	gl::drawTransparent = true;
	drawable.draw();

	// the world

	opaqueRenderList.clear();
	transp1RenderList.clear();
	transp2RenderList.clear();

	// Start thread to build render list
	pthread_t thr;
	threadFinished = false;
	if (pthread_create(&thr, NULL, buildListThread, NULL) != 0)
		cout << "pthread failed\n";

	// While we're waiting, load requested objects
	while (!threadFinished)
		objMan.loadSingleObject();

	// This shouldn't really be necessary, but somehow pthread_create
	// fails at some point when I don't put this line here.
	pthread_join(thr, NULL);

	// Everything is done now, draw!
	if (doBFC)
		glEnable(GL_CULL_FACE);
	renderOpaque();	// this constructs the transp1/2 render lists
	glDepthMask(GL_FALSE);
	renderTransp1();
	glDepthMask(GL_TRUE);
	glDisable(GL_CULL_FACE);
	water.draw();
	if (doBFC)
		glEnable(GL_CULL_FACE);
	renderTransp2();
	glDisable(GL_CULL_FACE);
}

void Renderer::init(void)
{
	doZones = false;
	doTextures = true;
	doFog = true;
	doVertexColors = true;
	doCol = false;
	doTrails = false;
	doBFC = false;
	lodMult = 1.0f;
}
