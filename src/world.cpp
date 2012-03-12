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

void World::drawIslands(void)
{
	quat camPos = cam.getPosition();
	float pos[3];
	pos[0] = camPos.x; pos[1] = camPos.y; pos[2] = camPos.z;

	for (uint i = 0; i < islands.size(); i++) {
		if (islands[i].pointInIsland(pos)) {
			activeIsland = i;
			break;
		}
	}

	islands[0].draw();
	islands[activeIsland].draw();
	for (uint i = 0; i < islands.size(); i++) {
		if (i != activeIsland)
			islands[i].drawLod();
	}

//	for (uint i = 0; i < islands.size(); i++)
//		islands[i].drawZones();
}

int ind;
vector<int> indices;

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

void World::readBinIpl(ifstream &in)
{
	in.seekg(4, ios::cur);
	uint instCount;
	in.read((char *) &instCount, sizeof(uint));
	in.seekg(68, ios::cur);

	Instance *ip;
	for (uint i = 0; i < instCount; i++) {
		ip = new Instance;
		in.read((char *) ip->position, 3*sizeof(float));
		in.read((char *) ip->rotation, 4*sizeof(float));
		in.read((char *) &ip->id, sizeof(uint));
		in.read((char *) &ip->interior, sizeof(uint));
		in.read((char *) &ip->lod, sizeof(uint));
		ip->scale[0] = 1.0f;
		ip->scale[1] = 1.0f;
		ip->scale[2] = 1.0f;

		ip->name = ((WorldObject*)objectList.get(ip->id))->modelName;

		ip->isIslandLod = ip->isLod = false;
		if (ip->name.substr(0,3) == "lod")
			ip->isLod = true;
		else if (ip->name.substr(0,9) == "islandlod")
			ip->isIslandLod = true;

		uint j;
		for (j = 0; j < islands.size(); j++)
			if (islands[j].pointInIsland(ip->position)) {
				if (ip->isIslandLod)
					islands[j].islandLods.
						      push_back(ip);
				else
					islands[j].instances.
						      push_back(ip);
				break;
			}
		if (j == islands.size()) {
			if (ip->isIslandLod)
				islands[0].islandLods.push_back(ip);
			else
				islands[0].instances.push_back(ip);
		}
		
		instances.push_back(ip);
	
		indices.push_back(ind++);
	}
}

void World::readTextIpl(ifstream &in)
{
	string line;
	vector<string> fields;
	int blockType = END;
	do {
		getline(in, line);
		getFields(line, ',', fields);
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

			if (hasInterior)
				ip->interior = atoi(fields[i++].c_str());
			else
				ip->interior = 0;

			ip->position[0] = atof(fields[i++].c_str());
			ip->position[1] = atof(fields[i++].c_str());
			ip->position[2] = atof(fields[i++].c_str());

			if (hasScale) {
				ip->scale[0] = atof(fields[i++].c_str());
				ip->scale[1] = atof(fields[i++].c_str());
				ip->scale[2] = atof(fields[i++].c_str());
			} else {
				ip->scale[0] = 1.0f;
				ip->scale[1] = 1.0f;
				ip->scale[2] = 1.0f;
			}
			ip->rotation[0] = atof(fields[i++].c_str());
			ip->rotation[1] = atof(fields[i++].c_str());
			ip->rotation[2] = atof(fields[i++].c_str());
			ip->rotation[3] = atof(fields[i++].c_str());

			if (hasLod)
				ip->lod = atoi(fields[i++].c_str());

			uint j;
			for (j = 0; j < islands.size(); j++)
				if (islands[j].pointInIsland(ip->position)) {
					if (ip->isIslandLod)
						islands[j].islandLods.
						              push_back(ip);
					else
						islands[j].instances.
						              push_back(ip);
					break;
				}
			if (j == islands.size()) {
				if (ip->isIslandLod)
					islands[0].islandLods.push_back(ip);
				else
					islands[0].instances.push_back(ip);
			}
			
			instances.push_back(ip);
		
			indices.push_back(ind++);
		} else if (blockType == ZONE) {
			int i = 0;
			Zone *z = new Zone;

			z->name = fields[i++];
			stringToLower(z->name);
			z->type = atoi(fields[i++].c_str());
			z->corner1[0] = atof(fields[i++].c_str());
			z->corner1[1] = atof(fields[i++].c_str());
			z->corner1[2] = atof(fields[i++].c_str());
			z->corner2[0] = atof(fields[i++].c_str());
			z->corner2[1] = atof(fields[i++].c_str());
			z->corner2[2] = atof(fields[i++].c_str());
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
		if (game == GTASA) {
			if (indices[i] == 0)
				base = i;
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

/*
	for (uint i = 0; i < ilods.size(); i++) {
		for (uint j = 0; j < islands.size(); j++) {
			if (islands[j].pointInIsland(ilods[i]->position)) {
				islands[j].lodIds.push_back(i);
				break;
			}
		}
	}
*/
}

int World::getLod(Instance *ip)
{
	string lodName = ip->name;
	lodName[0] = 'l'; lodName[1] = 'o'; lodName[2] = 'd';
	if (lodName == ip->name)
		return -1;
	for (uint i = 0; i < instances.size(); i++) {
		if (instances[i]->name == lodName)
			return i;
	}
	return -1;
}

void World::dump(void)
{
}

void Island::draw(void)
{
	for (uint i = 0; i < instances.size(); i++) {
		Instance *ip = instances[i];
		WorldObject *op = (WorldObject*)objectList.get(ip->id);

		glm::mat4 save = gl::modelMat;

		if (cam.distanceTo(quat(ip->position[0],
		                        ip->position[1],
		                        ip->position[2])) //>= 300)
	            >= op->drawDistances[0])
			continue;
//		if (!ip->isLod)
//			continue;

		ip->transform();

		if (!op->isLoaded)
			op->load();
		op->drawable.draw(false);
		op->drawable.draw(true);

		gl::modelMat = save;
		glm::mat4 modelView = gl::viewMat * gl::modelMat;
		glm::mat3 normal = glm::inverseTranspose(glm::mat3(modelView));
		glUniformMatrix4fv(gl::u_ModelView, 1, GL_FALSE,
				   glm::value_ptr(modelView));
		glUniformMatrix3fv(gl::u_NormalMat, 1, GL_FALSE,
				   glm::value_ptr(normal));
	}
}

void Island::drawLod(void)
{
	for (uint i = 0; i < islandLods.size(); i++) {
		Instance *ip = islandLods[i];
		WorldObject *op = (WorldObject*)objectList.get(ip->id);

		glm::mat4 save = gl::modelMat;

		ip->transform();

		if (!op->isLoaded)
			op->load();
		op->drawable.draw(false);
		op->drawable.draw(true);

		gl::modelMat = save;
		glm::mat4 modelView = gl::viewMat * gl::modelMat;
		glm::mat3 normal = glm::inverseTranspose(glm::mat3(modelView));
		glUniformMatrix4fv(gl::u_ModelView, 1, GL_FALSE,
				   glm::value_ptr(modelView));
		glUniformMatrix3fv(gl::u_NormalMat, 1, GL_FALSE,
				   glm::value_ptr(normal));
	}
	draw();
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
bool Island::pointInIsland(float *p)
{
	for (uint i = 0; i < islandZones.size(); i++)
		if (islandZones[i]->pointInZone(p))
			return true;
	return false;
}

bool Zone::pointInZone(float *p)
{
	if (p[0] >= corner1[0] && p[0] <= corner2[0] &&
	    p[1] >= corner1[1] && p[1] <= corner2[1] &&
	    p[2] >= corner1[2] && p[2] <= corner2[2]) {
		return true;
	}
	return false;
}

void Instance::transform(void)
{
	glm::quat q;
	q.x = -rotation[0];
	q.y = -rotation[1];
	q.z = -rotation[2];
	q.w = rotation[3];
	gl::modelMat = glm::translate(gl::modelMat,
	                      glm::vec3(position[0], position[1], position[2]));
	gl::modelMat *= glm::mat4_cast(q);
	glm::mat4 modelView = gl::viewMat * gl::modelMat;
	glm::mat3 normal = glm::inverseTranspose(glm::mat3(modelView));
	glUniformMatrix4fv(gl::u_ModelView, 1, GL_FALSE,
	                   glm::value_ptr(modelView));
	glUniformMatrix3fv(gl::u_NormalMat, 1, GL_FALSE,
	                   glm::value_ptr(normal));
}
