#include <cstdio>

#include <iostream>
#include <fstream>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "gta.h"
#include "math.h"
#include "gl.h"
#include "pipeline.h"
#include "primitives.h"
#include "col.h"
#include "camera.h"
#include "timecycle.h"
#include "directory.h"
#include "renderer.h"
#include "objects.h"
#include "world.h"

using namespace std;

World *world;

/*
 * WorldSector
 */

enum Intersection WorldSector::intersectBox(quat bmin, quat bmax)
{
	bmin.z = 0.0f;
	bmax.z = 0.0f;
	quat tmin = min;
	quat tmax = max;
	tmin.z = 0.0f;
	tmax.z = 0.0f;
	if (isPointInBox(bmin,tmin,tmax) && isPointInBox(bmax,tmin,tmax))
		return INSIDE;
	if (isPointInBox(bmin,tmin,tmax) && !isPointInBox(bmax,tmin,tmax))
		return INTERSECT;
	if (!isPointInBox(bmin,tmin,tmax) && isPointInBox(bmax,tmin,tmax))
		return INTERSECT;
	if (isPointInBox(min, bmin, bmax) && isPointInBox(max, bmin, bmax))
		return INTERSECT;
	return OUTSIDE;
}

bool WorldSector::isPointInside(quat p)
{
	return isPointInBox(p, min, max);
}

void WorldSector::addInstance(Instance *ip)
{
	instances.push_back(ip);
}

void WorldSector::setBounds(quat c1, quat c2)
{
	min = c1;
	max = c2;
}

quat WorldSector::getMinCorner(void)
{
	return min;
}

quat WorldSector::getMaxCorner(void)
{
	return max;
}

void WorldSector::addToRenderList(void)
{
	for (uint i = 0; i < instances.size(); i++)
		instances[i]->addToRenderList();
}

void WorldSector::draw(void)
{
	THREADCHECK();
	glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, gl::whiteTex);

	glVertexAttrib4f(gl::in_Color, 0.8f, 0.8f, 0.8f, 1.0f);
	gl::drawCube2(min, max);
}

/*
 * World
 */

void World::buildRenderList(void)
{
/*
	quat camPos = cam->getPosition();

	for (uint i = 0; i < islands.size(); i++) {
		if (islands[i].pointInIsland(camPos)) {
			activeIsland = i;
			break;
		}
	}

	islands[0].addToRenderList();
	islands[activeIsland].addToRenderList();
	for (uint i = 0; i < islands.size(); i++) {
		if (i != activeIsland)
			islands[i].addLodToRenderList();
	}
*/

	sectors[0].addToRenderList();
	for (uint i = 1; i < sectors.size(); i++)
		if (cam->isBoxInFrustum(sectors[i].getMinCorner(),
		                       sectors[i].getMaxCorner()))
			sectors[i].addToRenderList();
}

void World::cleanUp(void)
{
	for (uint i = 0; i < instances.size(); i++)
		instances[i]->releaseIfInactive();
//	texMan.dumpLoaded();
}

void World::drawZones(void)
{
	for (uint i = 0; i < islands.size(); i++)
		islands[i].drawZones();
}

static int ind;
static vector<int> indices;

void World::readIpl(ifstream &in, string iplName)
{
	ind = 0;
	readTextIpl(in);

	iplName.resize(iplName.length()-4);
	iplName += "_stream";

	char num[4];
	ifstream binFile;
	string binName;
	int i = 0;
	sprintf(num, "%d", i);
	binName = iplName + num + ".ipl";
	while (directory.openFile(binFile, binName) == 0) {
		readBinIpl(binFile);
		binFile.close();
		i++;
		sprintf(num, "%d", i);
		binName = iplName + num + ".ipl";
	}
}

void World::addInstance(Instance *i)
{
	instances.push_back(i);
	uint k = instances.size()-1;
	instances[k]->index = k;
	indices.push_back(ind++);
}

void World::populateIslands(void)
{
	for (uint i = 0; i < instances.size(); i++) {
		addInstanceToIsland(instances[i]);
		addInstanceToSectors(instances[i]);
	}
}

void World::addInstanceToSectors(Instance *ip)
{
	if (ip->isLod || ip->isIslandLod)
		return;

	bool added = false;
	CollisionModel *col = objectList->get(ip->id)->col;
	if (col == 0) {
//		cout << ip->name << " has no col\n";
		sectors[0].addInstance(ip);
		return;
	}
	// Lets hope rotation and scale don't mess this up.
	quat p1 = col->min + ip->position;
	quat p2 = col->max + ip->position;
	// Add an instance to a sector if it's inside or intersecting.
	for (uint i = 1; i < sectors.size(); i++) {
		if (sectors[i].intersectBox(p1, p2) != OUTSIDE) {
			sectors[i].addInstance(ip);
			added = true;
		}
	}
	if (!added) {
		sectors[0].addInstance(ip);
		cout << ip->name << " doesn't lie in any sector\n";
		cout << ip->position << endl;
		cout << col->min << " " << col->max << endl;
	}
}

void World::addInstanceToIsland(Instance *i)
{
	if (i->isLod)
		return;

	// TODO: this is probably false
	float dist = 120.0f;
	if (game == GTASA)
		dist = 500.0f;
	// add instance to the appropriate island; if none is found,
	// add to the default island (0)
	uint j;
	for (j = 1; j < islands.size(); j++)
		if (i->isIslandLod) {
			if (islands[j].pointInIsland(i->position)) {
				islands[j].islandLods.push_back(i);
				break;
			}
		} else {
			if (islands[j].sphereInIsland(i->position, dist)) {
				islands[j].instances.push_back(i);
				break;
			}
		}
	if (j == islands.size()) {
		if (i->isIslandLod) {
			cout << i->name << " not inserted\n";
			islands[0].islandLods.push_back(i);
		} else
			islands[0].instances.push_back(i);
	}
}

void World::readBinIpl(ifstream &in)
{
	in.seekg(4, ios::cur);
	uint instCount;
	in.read(reinterpret_cast<char*>(&instCount), sizeof(uint));
	in.seekg(68, ios::cur);

	for (uint i = 0; i < instCount; i++) {
		Instance *ip = new Instance;
		char data[40];
		in.read(data, 40);
		ip->initFromBinEntry(data);
		addInstance(ip);
	}
}

void World::readTextIpl(ifstream &in)
{
	string line;
	vector<string> fields;
	int blockType = END;
	do {
		getline(in, line);
		getFields(line, ",", fields);
		if (fields.size() < 1 || fields[0][0] == '#')
			continue;

		if (fields[0] == "inst") {
			blockType = INST;
			continue;
		} else if (fields[0] == "zone") {
			blockType = ZONE;
			continue;
		} else if (fields[0] == "cull") {
			blockType = CULL;
			continue;
		} else if (fields[0] == "pick") {
			blockType = PICK;
			continue;
		} else if (fields[0] == "path") {
			blockType = PATH_IPL;
			continue;
		} else if (fields[0] == "end") {
			blockType = END;
			continue;
		}

		if (blockType == INST) {
			Instance *ip = new Instance;
			ip->initFromLine(fields);
			addInstance(ip);
		} else if (blockType == ZONE) {
			Zone *z = new Zone;
			z->initFromLine(fields);

			if (uint(z->islandNum) >= islands.size())
				islands.resize(z->islandNum+1);

			if (z->type == 3)
				islands[z->islandNum].islandZones.push_back(z);
			else
				islands[z->islandNum].zones.push_back(z);
		}
	} while(!in.eof());
}

void World::associateLods(void)
{
	int base = 0;

	/* Associate every instance with its LOD and vice versa */
	for (uint i = 0; i < instances.size(); i++) {
		Instance *ip = instances[i];
		if (indices[i] == 0)
			base = i;
		if (ip->isLod || ip->isIslandLod)
			continue;
		if (game == GTASA) {
			if (ip->lod != -1) {
				ip->lod = base + ip->lod;
				instances[ip->lod]->hires.push_back(i);
				instances[ip->lod]->isLod = true;
			}
		} else {
			ip->lod = getLod(ip);
			if (ip->lod != -1)
				instances[ip->lod]->hires.push_back(i);
		}
	}

	// Some LODs in 3/VC don't have hires instances, add dummies
	WorldObject *dummyObj;
	dummyObj = new WorldObject;
	dummyObj->col = 0;
	dummyObj->type = OBJS;
	dummyObj->id = objectList->getObjectCount()-1;
	dummyObj->modelName = "LODDUMMY";
	dummyObj->textureName = "LODDUMMY";
	dummyObj->objectCount = 1;
	if (game == GTA3)
		// not sure about that one
		dummyObj->drawDistances.push_back(100);
	else
		dummyObj->drawDistances.push_back(0);
	dummyObj->flags = 0;
	objectList->add(dummyObj);

	for (uint i = 0; i < instances.size(); i++) {
		Instance *lod = instances[i];
		if (!lod->isLod || lod->hires.size() != 0)
			continue;
//		cout << "adding dummy for " << lod->name << endl;

		Instance *ip = new Instance;

		ip->lod = i;
		ip->id = dummyObj->id;
		string name = lod->name;
		name[0] = 'x'; name[1] = 'x'; name[2] = 'x';
		ip->name = name;
		ip->position = lod->position;
		ip->scale = quat(1.0f, 1.0f, 1.0f);
		ip->rotation = quat(1.0f, 0.0f, 0.0f, 0.0f);

		addInstance(ip);

		uint ind = instances.size()-1;
		lod->hires.push_back(ind);
	}

	indices.resize(0);
}

void World::initSectors(quat start, quat end, int count)
{
	start.z = -10000.0f;
	quat stepx((end.x - start.x)/count, 0.0f, 0.0f);
	quat stepy(0.0f, (end.y - start.y)/count, 0.0f);
	quat step(stepx.x, stepy.y, 20000.0f);

	sectors.resize(count*count+1);
	sectors[0].setBounds(quat(-10000.0f, -10000.0f, -10000.0f),
	                     quat(10000.0f, 10000.0f, 10000.0f));
#define S(i,j) sectors[i*count+j+1]
	for (int i = 0; i < count; i++) {
		quat base = start + stepx*i;
		for (int j = 0; j < count; j++) {
			S(i,j).setBounds(base, base+step);
			base += stepy;
		}
	}
#undef S
}

void World::drawSectors(void)
{
	for (uint i = 1; i < sectors.size(); i++)
		sectors[i].draw();
}

/* finds LOD instance for 3/VC LOD system and returns index */
int World::getLod(Instance *ip)
{
	// regular LOD name
	string lodName1 = ip->name;
	lodName1[0] = 'l'; lodName1[1] = 'o'; lodName1[2] = 'd';
	// some objects have LODs without '_dy' and '_nt' suffix
	string lodName2 = lodName1;
	if (lodName2.size() >= 3) {
		if (lodName2.substr(lodName2.size()-3, 3) == "_nt")
			lodName2.resize(lodName2.size()-3);
		else if (lodName2.substr(lodName2.size()-3, 3) == "_dy")
			lodName2.resize(lodName2.size()-3);
	}

	// look for the LOD with the least distance
	float bestD = 1000000;
	int bestLod = -1;

	for (uint i = 0; i < instances.size(); i++) {
		Instance *inst = instances[i];
		if (inst->name == lodName1 || inst->name == lodName1) {
			float d =
			    (inst->position - ip->position).normsq();
			if (d <= bestD) {
				bestD = d;
				bestLod = i;
			}
			// there probably won't be better matches
			if (bestD <= 100)
				return bestLod;
		}
	}
	return bestLod;
}

Instance *World::getInstance(uint i)
{
	if (i < instances.size())
		return instances[i];
	return 0;
}

void World::setInterior(int i)
{
	interior = i;
}

int World::getInterior(void)
{
	return interior;
}

World::World(void)
{
	interior = 0;
}

/*
 * Island
 */

void Island::addToRenderList(void)
{
	for (uint i = 0; i < instances.size(); i++)
		instances[i]->addToRenderList();
}

void Island::addLodToRenderList(void)
{
	for (uint i = 0; i < islandLods.size(); i++)
		islandLods[i]->addToRenderList();
}

void Island::drawZones(void)
{
	THREADCHECK();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glAlphaFunc(GL_GREATER, 0.0);
	glBindTexture(GL_TEXTURE_2D, gl::whiteTex);

	for (uint i = 0; i < islandZones.size(); i++) {
		if (islandZones[i]->islandNum == 1)
			glVertexAttrib4f(gl::in_Color, 1.0f, 0.0f, 0.0f, 0.5f);
		else if (islandZones[i]->islandNum == 2)
			glVertexAttrib4f(gl::in_Color, 0.0f, 1.0f, 0.0f, 0.5f);
		else if (islandZones[i]->islandNum == 3)
			glVertexAttrib4f(gl::in_Color, 0.0f, 0.0f, 1.0f, 0.5f);
		gl::drawCube2(islandZones[i]->corner1,
		              islandZones[i]->corner2);
	}

	glDisable(GL_BLEND);
}

bool Island::sphereInIsland(quat p, float r)
{
	for (uint i = 0; i < islandZones.size(); i++)
		if (islandZones[i]->sphereInZone(p, r))
			return true;
	return false;
}

bool Island::pointInIsland(quat p)
{
	for (uint i = 0; i < islandZones.size(); i++)
		if (islandZones[i]->pointInZone(p))
			return true;
	return false;
}

/*
 * Zone
 */

void Zone::initFromLine(std::vector<std::string> fields)
{
	int i = 0;
	name = fields[i++];
	stringToLower(name);
	type = atoi(fields[i++].c_str());
	corner1 = quat(atof(fields[i].c_str()),
	               atof(fields[i+1].c_str()),
	               atof(fields[i+2].c_str()));
	corner2 = quat(atof(fields[i+3].c_str()),
	               atof(fields[i+4].c_str()),
	               atof(fields[i+5].c_str()));
	i += 6;
	islandNum = atoi(fields[i++].c_str());

	if (fields.size() >= 10) {
		text = fields[i++];
		stringToLower(text);
	}
}

bool Zone::sphereInZone(quat p, float r)
{
	p.w = r;
	return isSphereInBox(p, corner1, corner2);
}

bool Zone::pointInZone(quat p)
{
	return isPointInBox(p, corner1, corner2);
}

/*
 * Instance
 */

void Instance::initFromLine(vector<string> fields)
{
	bool lineHasInterior = false;
	bool lineHasScale = false;
	bool lineHasLod = false;
	if (fields.size() == 12) { // gta3
		lineHasScale = true;
	} else if (fields.size() == 13) { // gtavc
		lineHasScale = true;
		lineHasInterior = true;
	} else if (fields.size() == 11) { // gtasa
		lineHasInterior = true;
		lineHasLod = true;
	}

	int i = 0;
	id = atoi(fields[i++].c_str());

	name = fields[i++];
	stringToLower(name);

	if (lineHasInterior)
		interior = atoi(fields[i++].c_str());

	position = quat(atof(fields[i].c_str()),
	                atof(fields[i+1].c_str()),
	                atof(fields[i+2].c_str()));
	i += 3;


	if (lineHasScale) {
		scale = quat(atof(fields[i].c_str()),
		             atof(fields[i+1].c_str()),
		             atof(fields[i+2].c_str()));
		i += 3;
	}
	rotation = quat(atof(fields[i+3].c_str()),
	                atof(fields[i+0].c_str()),
	                atof(fields[i+1].c_str()),
	                atof(fields[i+2].c_str()));
	i += 4;

	if (lineHasLod)
		lod = atoi(fields[i++].c_str());

	if (name.substr(0,3) == "lod")
		isLod = true;
	else if (name.substr(0,9) == "islandlod")
		isIslandLod = true;
	// Assume only non-lods for San Andreas, the flag is later corrected.
	if (game == GTASA)
		isIslandLod = isLod = false;
	// LODDUMMY
	if (name.substr(0,3) == "xxx")
		isIslandLod = isLod = false;
}

void Instance::initFromBinEntry(char *data)
{
	float *pos = reinterpret_cast<float *> (&data[0]);
	float *rot = reinterpret_cast<float *> (&data[12]);
	int *ints = reinterpret_cast<int *> (&data[28]);

	position = quat(pos[0], pos[1], pos[2]);
	rotation = quat(rot[3], rot[0], rot[1], rot[2]);
	id = ints[0];
	interior = ints[1];
	lod = ints[2];

	name = static_cast<WorldObject*>(objectList->get(id))->modelName;

	if (name.substr(0,3) == "lod")
		isLod = true;
	else if (name.substr(0,9) == "islandlod")
		isIslandLod = true;
	// Assume only non-lods for San Andreas, the flag is later corrected.
	if (game == GTASA)
		isIslandLod = isLod = false;
	// LODDUMMY
	if (name.substr(0,3) == "xxx")
		isIslandLod = isLod = false;
}

void Instance::addToRenderList(void)
{
	const float fadeThrshd = 20.0f;

	// set by 'addOpaqueObject'
	if (wasAdded)
		return;

	WorldObject *op = static_cast<WorldObject*>(objectList->get(id));

	float dist = cam->distanceTo(position)/renderer->lodMult;
	int ai = op->getCorrectAtomic(dist);

	// frustum cull
	if (op->canDraw())
		if (isCulled() && !(op->flags & 128))
			return;

	// if the instance is too far away, try the LOD version
	if (ai < 0) {
		addLodToRenderList();
		goto release;
	}

	if (isHidden)
		return;

	if (op->modelName == "LODDUMMY")
		return;

	// check for interior (if intr < 0 everything is drawn)
	int intr;
	intr = world->getInterior();
	if (interior != intr && intr >= 0 && !renderer->doCol) {
		if (game == GTAVC) {
			if (interior != 13)
				goto release;
		} if (game == GTASA) {
			// Gotta figure out what's the meaning of this.
			if (interior != 13 && interior != 256 &&
			    interior != 269 && interior != 2048 &&
			    interior != 768 && interior != 512 &&
			    interior != 6144 &&
			    interior != 1024 && interior != 4096)
				goto release;
		}
	}

	// check for time
	if (op->isTimed && !op->isVisibleAtTime(timeCycle.getHour()) &&
	    !renderer->doCol)
		goto release;

	// request model
	if (!isActive) {
		op->incRefCount();
		isActive = true;
	}

	if (!op->canDraw()) {
		addLodToRenderList();
		return;
	}

	float drawDist, distDiff;
	drawDist = op->getDrawDistance(ai);
	distDiff = drawDist - dist;

	if (distDiff < fadeThrshd)
		setFading(distDiff/fadeThrshd);
	else if (getFading() != 1.0f)
		// Increment fading value for instances that are
		// well within the draw dist.
		setFading(getFading()+0.25f);
	else if (op->isFreshlyLoaded)
		setFading(0.0f);
		// Retain the freshlyLoaded flag for other instances.
		// It is cleared by Renderer.
	else
		setFading(1.0f);

	// If a new instance is being faded in or out, also draw LOD
	if (getFading() != 1.0f)
		addLodToRenderList();

	if (!isActive)
		cout << name << " this can't be\n";

	if (op->type == ANIM)
		renderer->addOpaqueObject(this, -1);
	else
		renderer->addOpaqueObject(this, ai);
	return;

release:
	if (isActive) {
		op->decRefCount();
		isActive = false;
	}
	return;
}

void Instance::addLodToRenderList(void)
{
	Instance *ip = world->getInstance(lod);
	if (ip != 0)
		ip->addToRenderList();
}

bool Instance::isCulled(void)
{
	WorldObject *op = static_cast<WorldObject*>(objectList->get(id));
	if (!op->col)
		return false;
	quat bs = op->col->boundingSphere;
	bs.w = 0;
	bs = rotation.getConjugate() * bs * rotation;
	bs += position;
	bs.w = op->col->boundingSphere.w;
	if (cam->isSphereInFrustum(bs))
		return false;
	return true;
}

void Instance::releaseIfInactive(void)
{
	WorldObject *op = static_cast<WorldObject*>(objectList->get(id));

	float dist = cam->distanceTo(position)/renderer->lodMult;
	int ai = op->getCorrectAtomic(dist);

	if (ai < 0 && isActive) {
		op->decRefCount();
		isActive = false;
	}
}

void Instance::transform(void)
{
	glm::quat q;

	q.w = rotation.w;
	q.x = -rotation.x;
	q.y = -rotation.y;
	q.z = -rotation.z;

	gl::state.modelView = glm::translate(gl::state.modelView,
	                         glm::vec3(position.x, position.y, position.z));
	gl::state.modelView *= glm::mat4_cast(q);
	gl::state.calculateNormalMat();
	gl::state.updateMatrices();
}

void Instance::setFading(float fade)
{
	fadingFactor = fade;
	if (fadingFactor < 0.0f)
		fadingFactor = 0.0f;
	else if (fadingFactor > 1.0f)
		fadingFactor = 1.0f;
}

float Instance::getFading(void)
{
	return fadingFactor;
}

void Instance::printInfo(void)
{
	cout << id << ", "
	     << name << ", "
	     << interior << ", "
	     << position.x << ", "
	     << position.y << ", "
	     << position.z << ", "
	     << scale.x << ", "
	     << scale.y << ", "
	     << scale.z << ", "
	     << rotation.x << ", "
	     << rotation.y << ", "
	     << rotation.z << ", "
	     << rotation.w << endl;
	WorldObject *op = static_cast<WorldObject*>(objectList->get(id));
	op->printInfo();
}

Instance::Instance(void)
{
	lod = -1;
	interior = 0;
	scale = quat(1.0f, 1.0f, 1.0f);
	isHidden = false;
	wasAdded = false;
	isActive = false;
	isIslandLod = isLod = false;
	fadingFactor = 1.0f;
}
