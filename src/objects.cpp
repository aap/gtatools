#include <cstdio>
#include <cstdlib>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gta.h"
#include "math.h"
#include "gl.h"
#include "pipeline.h"
#include "primitives.h"
#include "directory.h"
#include "drawable.h"
#include "col.h"
#include "texman.h"
#include "jobqueue.h"
#include "renderer.h"
#include "objects.h"

using namespace std;

ObjectList *objectList;

/*
 * ObjectList
 */

void ObjectList::readIde(ifstream &in)
{
	string line;
	vector<string> fields;
	int blockType = END;
	do {
		getline(in, line);
		getFields(line, ",", fields);
		if (fields.size() < 1 || line[0] == '#')
			continue;

		if (fields[0] == "objs") {
			blockType = OBJS;
			continue;
		} else if (fields[0] == "tobj") {
			blockType = TOBJ;
			continue;
		} else if (fields[0] == "path") {
			blockType = PATH_IDE;
			continue;
		} else if (fields[0] == "2dfx") {
			blockType = TWODFX;
			continue;
		} else if (fields[0] == "anim") {
			blockType = ANIM;
			continue;
		} else if (fields[0] == "txdp") {
			blockType = TXDP;
			continue;
		} else if (fields[0] == "hier") {
			blockType = HIER;
			continue;
		} else if (fields[0] == "cars") {
			blockType = CARS;
			continue;
		} else if (fields[0] == "peds") {
			blockType = PEDS;
			continue;
		} else if (fields[0] == "weap") {
			blockType = WEAP;
			continue;
		} else if (fields[0] == "end") {
			blockType = END;
			continue;
		}

		if (blockType == OBJS || blockType == TOBJ ||
		    blockType == ANIM) {
			WorldObject *obj;
			obj = new WorldObject;
			obj->initFromLine(fields, blockType);
			add(obj);
		} else if (blockType == PEDS) {
			Ped *obj;
			obj = new Ped;
			obj->initFromLine(fields);
			add(obj);
		} else if (blockType == CARS) {
			Car *obj;
			obj = new Car;
			obj->initFromLine(fields);
			add(obj);
		} else if (blockType == TXDP) {
			int i = 0;
			string child = fields[i++];
			string parent = fields[i++];
			stringToLower(child);
			stringToLower(parent);
			child += ".txd";
			parent += ".txd";
			texMan->addParentInfo(child, parent);
		}
	} while(!in.eof());
}

void ObjectList::findAndReadCol(string fileName)
{
	stringToLower(fileName);
	fileName = fileName.substr(0, fileName.size()-4);

	ifstream f;
	if (directory->openFile(f, fileName+".col") == -1) {
		string fileName2;
		char num[4];
		int i = 1;
		sprintf(num, "_%d", i);
		fileName2 = fileName + num + ".col";
		while (directory->openFile(f, fileName2) == 0) {
			readCol(f);
			f.close();
			i++;
			sprintf(num, "_%d", i);
			fileName2 = fileName + num + ".col";
		}
		i = 1;
		sprintf(num, "%d", i);
		fileName2 = fileName + num + ".col";
		while (directory->openFile(f, fileName2) == 0) {
			readCol(f);
			f.close();
			i++;
			sprintf(num, "%d", i);
			fileName2 = fileName + num + ".col";
		}
	} else {
		readCol(f);
		f.close();
	}
}

void ObjectList::readCol(ifstream &in, int island)
{
	CollisionModel *col;
	while (!in.eof()) {
		col = new CollisionModel;
		if (col->read(in)) {
			delete col;
			break;
		}
		col->island = island;
		cols.push_back(col);
	}
}

void ObjectList::associateCols(void)
{
	Model *mp;
	for (size_t i = 0; i < cols.size(); i++) {
		mp = get(cols[i]->name);
		if (mp == 0) {
//			cout << "no model found for col "<<cols[i]->name<<endl;
		} else {
			mp->col = cols[i];
		}
	}
}

void ObjectList::dumpCols(void)
{
	for (size_t i = 0; i < cols.size(); i++)
		cout << cols[i]->name << endl;
}

void ObjectList::dump(void)
{
	for (int i = 0; i < objectCount; i++)
		if (objects[i]) {
			cout << objects[i]->modelName << endl;
			if (objects[i]->col)
				cout << "col " << objects[i]->col->name << endl;
		}
}

/*
Model *ObjectList::get(int i)
{
	if (i < objectCount && i >= 0)
		return objects[i];
	return 0;
}

Model *ObjectList::get(string name)
{
	for (int i = 0; i < objectCount; i++)
		if (objects[i] && objects[i]->modelName == name)
			return objects[i];
	return 0;
}
*/

void ObjectList::add(Model *o)
{
	if (int(o->id) >= objectCount) {
		cout << "warning: id " << o->id << " out of range\n";
		return;
	}
	objects[o->id] = o;
}

int ObjectList::getObjectCount(void)
{
	return objectCount;
}

ObjectList::ObjectList(int objs)
{
	objects = new Model*[objs];
	for (int i = 0; i < objs; i++)
		objects[i] = 0;
	objectCount = objs;
}

ObjectList::~ObjectList(void)
{
//	cout << "deleting objlist\n";
	for (int i = 0; i < objectCount; i++)
		delete objects[i];
	delete[] objects;
	for (size_t i = 0; i < cols.size(); i++)
		delete cols[i];
	cols.clear();
	objectCount = 0;
	objects = 0;
}

/*
 * WorldObject
 */

void WorldObject::initFromLine(std::vector<std::string> fields, int blockType)
{
	bool lineHasObjectCount = true;
	if (fields.size() == 5 || (fields.size() == 7 && blockType == TOBJ) ||
	    blockType == ANIM)
		lineHasObjectCount = false;

	int i = 0;
	type = blockType;
	id = atoi(fields[i++].c_str());
	modelName = fields[i++];
	textureName = fields[i++];
	stringToLower(modelName);
	stringToLower(textureName);

	if (blockType == ANIM) {
		isAnimated = true;
		animationName = fields[i++];
		stringToLower(animationName);
	}

	objectCount = 1;
	if (lineHasObjectCount)
		objectCount = atoi(fields[i++].c_str());

//	if(objectCount > 1)
//		cout << objectCount << " " << modelName << endl;

	for (uint j = 0; j < objectCount; j++) {
		float d = atof(fields[i++].c_str());
//		d *= d;
		drawDistances.push_back(d);
	}

	flags = atoi(fields[i++].c_str());

	isTimed = false;
	if (blockType == TOBJ) {
		timeOn = atoi(fields[i++].c_str());
		timeOff = atoi(fields[i++].c_str());
		isTimed = true;
	}
}

int WorldObject::getCorrectAtomic(float d)
{
	for (size_t i = 0; i < drawDistances.size(); i++) {
		if (d <= drawDistances[i])
			return i;
	}
	return -1;
}

float WorldObject::getDrawDistance(int atomic)
{
	if (atomic < 0)
		return drawDistances[drawDistances.size()-1];
	return drawDistances[atomic];
}

bool WorldObject::isVisibleAtTime(int hour)
{
	if (timeOn < timeOff) {	/* Day object */
		if (hour < timeOn ||
		    hour >= timeOff)
			return false;
		else
			return true;
	} else {		/* Night object */
		if (hour < timeOff ||
		    hour >= timeOn)
			return true;
		else
			return false;
	}
}

void WorldObject::printInfo(void)
{
	cout << id << ", "
	     << modelName << ", "
	     << textureName << ", ";
	if (type == ANIM)
		cout << animationName << ", ";
	cout << drawDistances.size() << ", ";
	for (size_t i = 0; i < drawDistances.size(); i++)
		cout << drawDistances[i] << ", ";
	cout << flags << ", ";
	if (type == TOBJ) {
		cout << timeOn << ", "
		     << timeOff << ", ";
	}
	cout << endl;
	cout << "hasCol: " << (col ? 1 : 0) << endl;
}

WorldObject::WorldObject(void)
{
	objectCount = 0;
	flags = 0;
	timeOn = 0;
	timeOff = 0;
	isTimed = false;
}

/*
 * Car
 */


void Car::initFromLine(std::vector<std::string> fields)
{
	int i = 0;
	type = CARS;
	id = atoi(fields[i++].c_str());
	modelName = fields[i++];
	textureName = fields[i++];
	vehicleType = fields[i++];
	handlingId = fields[i++];
	gameName = fields[i++];
	if (game == GTAVC || game == GTASA)
		anims = fields[i++];
	className = fields[i++];
	frequency = atoi(fields[i++].c_str());
	level = atoi(fields[i++].c_str());
	compRules = atoi(fields[i++].c_str());

	stringToLower(modelName);
	stringToLower(textureName);
	stringToLower(vehicleType);
	stringToLower(handlingId);
	stringToLower(anims);
	stringToLower(className);

	if (game != GTASA && className == "car") {
		wheelModelId=atoi(fields[i++].c_str());
		wheelScaleF = atof(fields[i++].c_str());
		wheelScaleR = wheelScaleF;
	}
	if (game != GTASA && className == "bike")
		unknown = atoi(fields[i++].c_str());
	if (game != GTASA && className == "plane")
		lodId = atoi(fields[i++].c_str());
}

/*
 * Model
 */

void Model::drawBoundingSphere(void)
{
	THREADCHECK();
	glm::mat4 mvSave = gl::state.modelView;
	glm::mat3 nrmSave = gl::state.normalMat;

	gl::state.modelView = glm::translate(gl::state.modelView,
	                                     glm::vec3(col->boundingSphere.x,
	                                               col->boundingSphere.y,
	                                               col->boundingSphere.z));
	// don't need normal matrix
	gl::state.updateMatrices();


	glBindTexture(GL_TEXTURE_2D, renderer->whiteTex);
	glVertexAttrib4f(gl::in_Color, 1.0f, 1.0f, 1.0f, 0.5f);
	glm::vec4 color;
	if (col->island == 0)
		color = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
	else if (col->island == 1)
		color = glm::vec4(0.8f, 0.0f, 0.0f, 1.0f);
	else if (col->island == 2)
		color = glm::vec4(0.0f, 0.8f, 0.0f, 1.0f);
	else if (col->island == 3)
		color = glm::vec4(0.0f, 0.0f, 0.8f, 1.0f);
	else
		color = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
	gl::state.matColor = color;
	gl::state.updateMaterial();

	gl::drawSphere(col->boundingSphere.w, 8, 8);

	gl::state.modelView = mvSave;
	gl::state.normalMat = nrmSave;
	gl::state.updateMatrices();
}

void Model::drawCol(void)
{
	THREADCHECK();
	// draw faces
	glBindTexture(GL_TEXTURE_2D, renderer->whiteTex);
	glVertexAttrib4f(gl::in_Color, 1.0f, 1.0f, 1.0f, 1.0f);

	vector<GLfloat> verts;
	GLuint vbo;
	for (size_t i = 0; i < col->faces.size(); i++) {
		verts.push_back(col->vertices[col->faces[i].a*3+0]);
		verts.push_back(col->vertices[col->faces[i].a*3+1]);
		verts.push_back(col->vertices[col->faces[i].a*3+2]);
		verts.push_back(col->vertices[col->faces[i].b*3+0]);
		verts.push_back(col->vertices[col->faces[i].b*3+1]);
		verts.push_back(col->vertices[col->faces[i].b*3+2]);
		verts.push_back(col->vertices[col->faces[i].c*3+0]);
		verts.push_back(col->vertices[col->faces[i].c*3+1]);
		verts.push_back(col->vertices[col->faces[i].c*3+2]);
	}
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(GLfloat),
	             &verts[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(gl::in_Vertex);
	glVertexAttribPointer(gl::in_Vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glm::vec4 color;

	if (col->island == 0)
		color = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
	else if (col->island == 1)
		color = glm::vec4(0.8f, 0.0f, 0.0f, 1.0f);
	else if (col->island == 2)
		color = glm::vec4(0.0f, 0.8f, 0.0f, 1.0f);
	else if (col->island == 3)
		color = glm::vec4(0.0f, 0.0f, 0.8f, 1.0f);
	else
		color = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
	gl::state.matColor = color;
	gl::state.updateMaterial();
	glDrawArrays(GL_TRIANGLES, 0, verts.size()/3);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	gl::state.matColor = glm::vec4(0.27f, 0.27f, 0.27f, 1.0f);
	gl::state.updateMaterial();
	glLineWidth(2);
	glDrawArrays(GL_TRIANGLES, 0, verts.size()/3);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glLineWidth(1);

	glDisableVertexAttribArray(gl::in_Vertex);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &vbo);

	// draw boxes
	for (size_t i = 0; i < col->boxes.size(); i++) {
		gl::state.matColor = color;
		gl::state.updateMaterial();
		gl::drawCube2(col->boxes[i].min, col->boxes[i].max);

		glLineWidth(2);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		gl::state.matColor = glm::vec4(0.27f, 0.27f, 0.27f, 1.0f);
		gl::state.updateMaterial();
		gl::drawCube2(col->boxes[i].min, col->boxes[i].max);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glLineWidth(1);
	}
}

void Model::load(void)
{
//	cout << "loading: " << modelName << " " << textureName << endl;
	if (drawable) {
		cout << "can't load two drawables (" << modelName << ")\n";
		return;
	}
	drawable = new Drawable;
	if (drawable == 0)
		cout << "panic: no more memory\n";
	drawable->request(modelName+".dff", textureName+".txd");
	isFreshlyLoaded = true;
}

void Model::loadSynch(void)
{
	drawable = new Drawable;
	drawable->loadSynch(modelName+".dff", textureName+".txd");
	isFreshlyLoaded = true;
}

void Model::unload(void)
{
	drawable->release();
	drawable = 0;
}

void Model::incRefCount(void)
{
	if (refCount == 0)
		load();
	refCount++;
}

void Model::decRefCount(void)
{
	refCount--;
	if (refCount == 0)
		unload();
}

bool Model::canDraw(void)
{
	return drawable != 0 && drawable->hasModel() &&
	       (drawable->hasTextures() || !renderer->doTextures);
}

Model::Model(void)
{
	BSvisible = false;
	isAnimated = false;
	isFreshlyLoaded = false;
	refCount = 0;
	drawable = 0;
	col = 0;
}

Model::~Model(void)
{
	if (drawable)
		delete drawable;
}
