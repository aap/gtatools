#include "gta.h"

#include <fstream>
#include <cstdlib>

#include <renderware.h>

#include <glm/gtc/quaternion.hpp>
#include "camera.h"
#include "directory.h"
#include "world.h"
#include "primitives.h"
#include "gl.h"

using namespace std;

World world;

/*
 * World
 */

void World::drawOpaque(void)
{
	quat camPos = cam.getPosition();

	for (uint i = 0; i < islands.size(); i++) {
		if (islands[i].pointInIsland(camPos)) {
			activeIsland = i;
			break;
		}
	}

	gl::drawTransparent = false;
	transpInstances.clear();
	islands[0].draw();
	islands[activeIsland].draw();
	for (uint i = 0; i < islands.size(); i++) {
		if (i != activeIsland) {
			islands[i].drawLod();
		}
	}
}

void World::drawTransparent(void)
{
	gl::drawTransparent = true;
	for (uint i = 0; i < transpInstances.size(); i++)
		transpInstances[i]->justDraw();
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
	i->isVisible = true;

	instances.push_back(i);
	uint k = instances.size()-1;
	instances[k]->index = k;
	indices.push_back(ind++);

	if (i->isLod)
		return;

	// TODO: tune this value
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
	in.read((char *) &instCount, sizeof(uint));
	in.seekg(68, ios::cur);

	Instance *ip;
	for (uint i = 0; i < instCount; i++) {
		ip = new Instance;
		float tmp[4];
		in.read((char *) tmp, 3*sizeof(float));
		ip->position = quat(tmp[0], tmp[1], tmp[2]);
		in.read((char *) tmp, 4*sizeof(float));
		ip->rotation = quat(tmp[3], tmp[0], tmp[1], tmp[2]);
		in.read((char *) &ip->id, sizeof(uint));
		in.read((char *) &ip->interior, sizeof(uint));
		in.read((char *) &ip->lod, sizeof(uint));
		ip->scale = quat(1.0f, 1.0f, 1.0f);

		ip->name = ((WorldObject*)objectList.get(ip->id))->modelName;

		ip->isIslandLod = ip->isLod = false;
		if (ip->name.substr(0,3) == "lod")
			ip->isLod = true;
		else if (ip->name.substr(0,9) == "islandlod")
			ip->isIslandLod = true;
		if (game == GTASA && ip->name.size() > 3) {
			if (ip->name.substr(ip->name.size()-3, 3) == "lod")
				ip->isLod = true;
		}

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

		Instance *ip;
		bool hasInterior = false;
		bool hasScale = false;
		bool hasLod = false;
		if (blockType == INST) {
			if (fields.size() == 12) { // gta3
				hasScale = true;
			} else if (fields.size() == 13) { // gtavc
				hasScale = true;
				hasInterior = true;
			} else if (fields.size() == 11) { // gtasa
				hasInterior = true;
				hasLod = true;
			}

			int i = 0;
			ip = new Instance;
			ip->lod = -1;

			ip->id = atoi(fields[i++].c_str());

			ip->name = fields[i++];
			stringToLower(ip->name);

			ip->isIslandLod = ip->isLod = false;
			if (ip->name.substr(0,3) == "lod")
				ip->isLod = true;
			else if (ip->name.substr(0,9) == "islandlod")
				ip->isIslandLod = true;
			if (game == GTASA && ip->name.size() > 3)
				if (ip->name.substr(ip->name.size()-3, 3)
				    == "lod")
					ip->isLod = true;

			if (hasInterior)
				ip->interior = atoi(fields[i++].c_str());
			else
				ip->interior = 0;

			ip->position = quat(atof(fields[i].c_str()),
			                    atof(fields[i+1].c_str()),
			                    atof(fields[i+2].c_str()));
			i += 3;


			if (hasScale) {
				ip->scale = quat(atof(fields[i].c_str()),
			                         atof(fields[i+1].c_str()),
			                         atof(fields[i+2].c_str()));
				i += 3;
			} else {
				ip->scale = quat(1.0f, 1.0f, 1.0f);
			}
			ip->rotation = quat(atof(fields[i+3].c_str()),
			                    atof(fields[i+0].c_str()),
			                    atof(fields[i+1].c_str()),
			                    atof(fields[i+2].c_str()));
			i += 4;

			if (hasLod)
				ip->lod = atoi(fields[i++].c_str());

			addInstance(ip);
		} else if (blockType == ZONE) {
			int i = 0;
			Zone *z = new Zone;

			z->name = fields[i++];
			stringToLower(z->name);
			z->type = atoi(fields[i++].c_str());
			z->corner1 = quat(atof(fields[i].c_str()),
			                  atof(fields[i+1].c_str()),
			                  atof(fields[i+2].c_str()));
			z->corner2 = quat(atof(fields[i+3].c_str()),
			                  atof(fields[i+4].c_str()),
			                  atof(fields[i+5].c_str()));
			i += 6;
			z->islandNum = atoi(fields[i++].c_str());

			if (fields.size() >= 10) {
				z->text = fields[i++];
				stringToLower(z->text);
			}

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

	/* Associate each instance with its LOD and vice versa */
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
			}
		} else {
			ip->lod = getLod(ip);
			if (ip->lod != -1)
				instances[ip->lod]->hires.push_back(i);
		}
	}

	/* Some LODs don't have hires instances, add dummies */
	// TODO: find out how the game handles these

	WorldObject *dummyObj;
	dummyObj = new WorldObject;
	dummyObj->type = OBJS;
	dummyObj->id = objectList.getObjectCount()-1;
	dummyObj->modelName = "LODDUMMY";
	dummyObj->textureName = "LODDUMMY";
	dummyObj->objectCount = 1;
	// not sure about that one
	dummyObj->drawDistances.push_back(100);
//	dummyObj->drawDistances.push_back(0);
	dummyObj->flags = 0;
	objectList.add(dummyObj);

	for (uint i = 0; i < instances.size(); i++) {
		Instance *lod = instances[i];
		if (!lod->isLod || lod->hires.size() != 0)
			continue;
//		cout << "adding dummy for " << lod->name << endl;

		Instance *ip = new Instance;
		ip->lod = i;
		ip->id = dummyObj->id;
		ip->name = lod->name;
		ip->name[0] = 'x'; ip->name[1] = 'x'; ip->name[2] = 'x';
		ip->isIslandLod = ip->isLod = false;
		ip->interior = 0;
		ip->position = lod->position;
		ip->scale = quat(1.0f, 1.0f, 1.0f);
		ip->rotation = quat(1.0f, 0.0f, 0.0f, 0.0f);

		addInstance(ip);

		uint ind = instances.size()-1;
		lod->hires.push_back(ind);
	}

	indices.resize(0);
}

/* finds LOD instance for 3/VC LOD system and returns index */
int World::getLod(Instance *ip)
{
	// regular LOD name
	string lodName1 = ip->name;
	lodName1[0] = 'l'; lodName1[1] = 'o'; lodName1[2] = 'd';
	// some objects have LODs without '_dy' and '_nt' suffix
	string lodName2 = lodName1;
	size_t pos;
	pos = lodName2.find("_nt");
	if (pos != string::npos) {
		lodName2.resize(lodName2.size()-3);
	} else {
		pos = lodName2.find("_dy");
		if (pos != string::npos)
			lodName2.resize(lodName2.size()-3);
	}

	// look for the LOD with the least distance
	float bestD = 1000000;
	int bestLod = -1;

	for (uint i = 0; i < instances.size(); i++) {
		Instance *inst = instances[i];
		if (inst->name == lodName1 || inst->name == lodName1) {
			float d = (inst->position - ip->position).normsq();
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
	if (i >= 0 && i < instances.size())
		return instances[i];
	return 0;
}

void World::addTransparent(Instance *ip, float dist)
{
	// TODO: depth sort
	transpInstances.push_back(ip);
}

void World::setInterior(int i) { interior = i; }
int World::getInterior(void) { return interior; }
int World::getHour(void) { return hour; }
int World::getMinute(void) { return minute; }
void World::setHour(int h)
{
	if (h < 0)
		hour = 23;
	else if (h >= 24)
		hour = 0;
	else
		hour = h;

	if (hour == 20)
		timeOfDay = 2;
	else if (hour == 6)
		timeOfDay = 0;
}

void World::setMinute(int m)
{
	if (m < 0) {
		minute = 59;
		setHour(hour-1);
	} else if (m >= 60) {
		minute = 0;
		setHour(hour+1);
	} else {
		minute = m;
	}
}

// 0: dawn
// 1: day
// 2: sunset
// 3: night
void World::setTimeOfDay(int t)
{
	if (t >= 0 && t <= 3)
		timeOfDay = t;
}

int World::getTimeOfDay(void) { return timeOfDay; }

World::World(void)
{
	interior = 0;
	hour = 12;
	minute = 0;
	timeOfDay = 1;
}

/*
 * Island
 */

void Island::draw(void)
{
	for (uint i = 0; i < instances.size(); i++)
		instances[i]->draw();
}

void Island::drawLod(void)
{
	for (uint i = 0; i < islandLods.size(); i++)
		islandLods[i]->draw();
}

void Island::drawZones(void)
{
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

bool Zone::sphereInZone(quat p, float r)
{
	if (p.x - r >= corner1.x && p.x + r <= corner2.x &&
	    p.y - r >= corner1.y && p.y + r <= corner2.y &&
	    p.z - r >= corner1.z && p.z + r <= corner2.z) {
		return true;
	}
	return false;
}

bool Zone::pointInZone(quat p)
{
	if (p.x >= corner1.x && p.x <= corner2.x &&
	    p.y >= corner1.y && p.y <= corner2.y &&
	    p.z >= corner1.z && p.z <= corner2.z) {
		return true;
	}
	return false;
}

/*
 * Instance
 */

void Instance::draw(void)
{
	WorldObject *op = (WorldObject*)objectList.get(id);

	glm::mat4 save = gl::modelMat;

	// check draw distance
	float d = cam.distanceTo(position);
	int ai = op->getCorrectAtomic(d);

	// frustum cull
	if (op->isLoaded)
		if (isCulled())
			return;

	if (op->isLoaded) {
		if (world.getTimeOfDay() == 0)
			op->drawable.setVertexColors(0);
		else if (world.getTimeOfDay() == 2)
			op->drawable.setVertexColors(1);
	}

	// if the instance is too far away, try the LOD version
	if (ai < 0) {
		Instance *ip = world.getInstance(lod);
		if (ip != 0)
			ip->draw();
		return;
	}

	if (!isVisible)
		return;

	if (op->modelName == "LODDUMMY")
		return;

	// check for interior (if intr < 0 everything is drawn)
	int intr = world.getInterior();
	if (interior != intr && intr >= 0) {
		if (game == GTAVC) {
			if (interior != 13)
				return;
		} if (game == GTASA) {
			if (interior != 13 && interior != 256 &&
			    interior != 269 && interior != 2048 &&
			    interior != 768 && interior != 512 &&
			    interior != 1024 && interior != 4096)
				return;
		}
	}

	// check for time
	if (op->isTimed && !op->isVisibleAtTime(world.getHour()))
		return;

	transform();

	glStencilFunc(GL_ALWAYS, (index>>gl::stencilShift)&0xFF, -1);

	if (!op->isLoaded)
		op->load();

	gl::wasTransparent = false;
	if (op->type == ANIM)
		op->drawable.draw();
	else
		op->drawable.drawAtomic(ai);

	// the flag from the item definition isn't reliable
	if (gl::wasTransparent)
		world.addTransparent(this, d);

	gl::modelMat = save;
	glm::mat4 modelView = gl::viewMat * gl::modelMat;
	glm::mat3 normal = glm::inverseTranspose(glm::mat3(modelView));
	glUniformMatrix4fv(gl::u_ModelView, 1, GL_FALSE,
			   glm::value_ptr(modelView));
	glUniformMatrix3fv(gl::u_NormalMat, 1, GL_FALSE,
			   glm::value_ptr(normal));
}

void Instance::justDraw(void)
{
	if (!isVisible)
		return;
	WorldObject *op = (WorldObject*)objectList.get(id);
	float d = cam.distanceTo(position);
	int ai = op->getCorrectAtomic(d);

	glm::mat4 save = gl::modelMat;
	transform();

	glStencilFunc(GL_ALWAYS, (index>>gl::stencilShift)&0xFF, -1);
	if (!op->isLoaded)
		op->load();
	if (op->type == ANIM)
		op->drawable.draw();
	else
		op->drawable.drawAtomic(ai);

	gl::modelMat = save;
	glm::mat4 modelView = gl::viewMat * gl::modelMat;
	glm::mat3 normal = glm::inverseTranspose(glm::mat3(modelView));
	glUniformMatrix4fv(gl::u_ModelView, 1, GL_FALSE,
			   glm::value_ptr(modelView));
	glUniformMatrix3fv(gl::u_NormalMat, 1, GL_FALSE,
			   glm::value_ptr(normal));
}

bool Instance::isCulled(void)
{
	WorldObject *op = (WorldObject*)objectList.get(id);

	for (uint i = 0; i < op->boundingSpheres.size(); i++)
		if (cam.isSphereInFrustum(op->boundingSpheres[i] + position))
			return false;
	return true;
}

void Instance::transform(void)
{
	glm::quat q;

	q.w = rotation.w;
	q.x = -rotation.x;
	q.y = -rotation.y;
	q.z = -rotation.z;
	gl::modelMat = glm::translate(gl::modelMat,
	                      glm::vec3(position.x, position.y, position.z));
	gl::modelMat *= glm::mat4_cast(q);
	glm::mat4 modelView = gl::viewMat * gl::modelMat;
	glm::mat3 normal = glm::inverseTranspose(glm::mat3(modelView));
	glUniformMatrix4fv(gl::u_ModelView, 1, GL_FALSE,
	                   glm::value_ptr(modelView));
	glUniformMatrix3fv(gl::u_NormalMat, 1, GL_FALSE,
	                   glm::value_ptr(normal));
}

void Instance::setVisible(bool v)
{
	isVisible = v;
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
	WorldObject *op = (WorldObject*) objectList.get(id);
	op->printInfo();
}
