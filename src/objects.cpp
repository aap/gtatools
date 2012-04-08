#include "gta.h"
#include <glm/gtc/quaternion.hpp>
#include "gl.h"
#include "directory.h"
#include "primitives.h"
#include "objects.h"
#include "world.h"
#include "texman.h"

using namespace std;

ObjectList objectList;

/*
 * ObjectList
 */

void ObjectList::readIde(ifstream &in)
{
	string line;
	vector<string> fields;
	int blockType = END;
	bool hasObjectCount = true;
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
			WorldObject *newObj;
			if (fields.size() == 5 ||
			    (fields.size() == 7 && blockType == TOBJ) ||
			    blockType == ANIM)
				hasObjectCount = false;

			int i = 0;
			newObj = new WorldObject;
			newObj->type = blockType;
			newObj->id = atoi(fields[i++].c_str());
			newObj->col = 0;
			newObj->BSvisible = false;
			newObj->modelName = fields[i++];
			newObj->textureName = fields[i++];
			stringToLower(newObj->modelName);
			stringToLower(newObj->textureName);

			if (blockType == ANIM) {
				newObj->isAnimated = true;
				newObj->animationName = fields[i++];
				stringToLower(newObj->animationName);
			}

			if (hasObjectCount)
				newObj->objectCount = atoi(fields[i++].c_str());
			else
				newObj->objectCount = 1;

			for (uint j = 0; j < newObj->objectCount; j++)
				newObj->drawDistances.push_back(
				                    atof(fields[i++].c_str()));

			newObj->flags = atoi(fields[i++].c_str());

			if (blockType == TOBJ) {
				newObj->timeOn = atoi(fields[i++].c_str());
				newObj->timeOff = atoi(fields[i++].c_str());
				newObj->isTimed = true;
			} else {
				newObj->isTimed = false;
			}

			add(newObj);
		} else if (blockType == PEDS) {
			Ped *newObj;
			int i = 0;
			newObj = new Ped;
			newObj->type = blockType;
			newObj->id = atoi(fields[i++].c_str());
			newObj->col = 0;
			newObj->BSvisible = false;
			newObj->modelName = fields[i++];
			newObj->textureName = fields[i++];
			newObj->defaultPedType = fields[i++];
			newObj->behaviour = fields[i++];
			newObj->animGroup = fields[i++];
			stringToLower(newObj->modelName);
			stringToLower(newObj->textureName);
			stringToLower(newObj->defaultPedType);
			stringToLower(newObj->behaviour);
			stringToLower(newObj->animGroup);
			newObj->carsCanDrive = atoi(fields[i++].c_str());

			if (fields.size() >= 10) {	// vc and sa
				newObj->animFile = fields[i++];
				stringToLower(newObj->animFile);
				newObj->radio1 = atoi(fields[i++].c_str());
				newObj->radio2 = atoi(fields[i++].c_str());
			}
			if (fields.size() >= 13) {	// sa
				newObj->voiceArchive = fields[i++];
				newObj->voice1 = fields[i++];
				newObj->voice2 = fields[i++];
				stringToLower(newObj->voiceArchive);
				stringToLower(newObj->voice1);
				stringToLower(newObj->voice2);
			}

			add(newObj);
		} else if (blockType == CARS) {
			Car *newObj;
			int i = 0;
			newObj = new Car;
			newObj->type = blockType;
			newObj->id = atoi(fields[i++].c_str());
			newObj->col = 0;
			newObj->BSvisible = false;
			newObj->modelName = fields[i++];
			newObj->textureName = fields[i++];
			newObj->vehicleType = fields[i++];
			newObj->handlingId = fields[i++];
			newObj->gameName = fields[i++];
			if (game == GTAVC || game == GTASA)
				newObj->anims = fields[i++];
			newObj->className = fields[i++];
			newObj->frequency = atoi(fields[i++].c_str());
			newObj->level = atoi(fields[i++].c_str());
			newObj->compRules = atoi(fields[i++].c_str());

			stringToLower(newObj->modelName);
			stringToLower(newObj->textureName);
			stringToLower(newObj->vehicleType);
			stringToLower(newObj->handlingId);
			stringToLower(newObj->anims);
			stringToLower(newObj->className);

			if (game != GTASA && newObj->className == "car") {
				newObj->wheelModelId=atoi(fields[i++].c_str());
				newObj->wheelScaleF = atof(fields[i++].c_str());
				newObj->wheelScaleR = newObj->wheelScaleF;
			}
			if (game != GTASA && newObj->className == "bike") {
				newObj->unknown = atoi(fields[i++].c_str());
			}
			if (game != GTASA && newObj->className == "plane") {
				newObj->lodId = atoi(fields[i++].c_str());
			}

			add(newObj);
		} else if (blockType == TXDP) {
			int i = 0;
			string child = fields[i++];
			string parent = fields[i++];
			stringToLower(child);
			stringToLower(parent);
			child += ".txd";
			parent += ".txd";
			texMan.addParentInfo(child, parent);
		}
	} while(!in.eof());
}

void ObjectList::findAndReadCol(string fileName)
{
	stringToLower(fileName);
	fileName = fileName.substr(0, fileName.size()-4);

	ifstream f;
	if (directory.openFile(f, fileName+".col") == -1) {
		string fileName2;
		char num[4];
		int i = 1;
		sprintf(num, "_%d", i);
		fileName2 = fileName + num + ".col";
		while (directory.openFile(f, fileName2) == 0) {
//			cout << "reading " << fileName2 << endl;
			readCol(f);
			f.close();
			i++;
			sprintf(num, "_%d", i);
			fileName2 = fileName + num + ".col";
		}
		i = 1;
		sprintf(num, "%d", i);
		fileName2 = fileName + num + ".col";
		while (directory.openFile(f, fileName2) == 0) {
//			cout << "reading " << fileName2 << endl;
			readCol(f);
			f.close();
			i++;
			sprintf(num, "%d", i);
			fileName2 = fileName + num + ".col";
		}
	} else {
//		cout << "reading " << fileName+".col" << endl;
		readCol(f);
		f.close();
	}
}

void ObjectList::readCol(ifstream &in, int island)
{
	CollisionModel *col;
	while (!in.eof()) {
		col = new CollisionModel;
		if (col->read(in) == 0) {
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
	for (uint i = 0; i < cols.size(); i++) {
		mp = get(cols[i]->name);
		if (mp == 0) {
			cout << "no model found for col "<<cols[i]->name<<endl;
		} else {
			mp->col = cols[i];
		}
	}
}

void ObjectList::dumpCols(void)
{
	for (uint i = 0; i < cols.size(); i++)
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

void ObjectList::add(Model *o)
{
	if ((int)o->id >= objectCount || o->id < 0) {
		cout << "warning: id " << o->id << " out of range\n";
		return;
	}
	objects[o->id] = o;
}

int ObjectList::getObjectCount(void)
{
	return objectCount;
}

void ObjectList::init(int objs)
{
	objects = new Model*[objs];
	for (int i = 0; i < objs; i++)
		objects[i] = 0;
	objectCount = objs;
}

ObjectList::ObjectList(void)
{
}

ObjectList::~ObjectList(void)
{
	for (int i = 0; i < objectCount; i++)
		delete objects[i];
	delete[] objects;
}

/*
 * WorldObject
 */

int WorldObject::getCorrectAtomic(float d)
{
	for (uint i = 0; i < drawDistances.size(); i++) {
		if (d <= drawDistances[i])
			return i;
	}
	return -1;
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
	for (uint i = 0; i < drawDistances.size(); i++)
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
	isTimed = false;
}

/*
 * Model
 */

void Model::drawBoundingSphere(void)
{
	glm::mat4 save = gl::modelMat;

	gl::modelMat = glm::translate(gl::modelMat,
	                      glm::vec3(col->boundingSphere.x,
	                                col->boundingSphere.y,
	                                col->boundingSphere.z));
	glm::mat4 modelView = gl::viewMat * gl::modelMat;
	glm::mat3 normal = glm::inverseTranspose(glm::mat3(modelView));

	gl::state.modelView = modelView;
	gl::state.normalMat = normal;
	gl::state.updateMatrices();

	glBindTexture(GL_TEXTURE_2D, gl::whiteTex);
	glVertexAttrib4f(gl::in_Color, 1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec4 color(0.8f, 0.8f, 0.8f, 1.0f);
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


	gl::modelMat = save;
	modelView = gl::viewMat * gl::modelMat;
	normal = glm::inverseTranspose(glm::mat3(modelView));

	gl::state.modelView = modelView;
	gl::state.normalMat = normal;
	gl::state.updateMatrices();
}

void Model::drawCol(void)
{
/*
	glm::mat4 save = gl::modelMat;

	gl::modelMat = glm::translate(gl::modelMat,
	                      glm::vec3(col->boundingSphere.x,
	                                col->boundingSphere.y,
	                                col->boundingSphere.z));
	glm::mat4 modelView = gl::viewMat * gl::modelMat;
	glm::mat3 normal = glm::inverseTranspose(glm::mat3(modelView));

	gl::state.modelView = modelView;
	gl::state.normalMat = normal;
//	gl::state.updateMatrices();
*/


	// draw faces
	glBindTexture(GL_TEXTURE_2D, gl::whiteTex);
	glVertexAttrib4f(gl::in_Color, 1.0f, 1.0f, 1.0f, 1.0f);

	vector<GLfloat> verts;
	GLuint vbo;
	for (uint i = 0; i < col->faces.size(); i++) {
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

	glm::vec4 color(0.8f, 0.8f, 0.8f, 1.0f);

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
	for (uint i = 0; i < col->boxes.size(); i++) {
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

/*
	gl::modelMat = save;
	modelView = gl::viewMat * gl::modelMat;
	normal = glm::inverseTranspose(glm::mat3(modelView));

	gl::state.modelView = modelView;
	gl::state.normalMat = normal;
//	gl::state.updateMatrices();
*/
}

void Model::load(void)
{
//	cout << modelName << " " << textureName << endl;
	if (drawable.load(modelName+".dff", textureName+".txd") == -1)
		return;
	boundingSpheres = drawable.getBoundingSpheres();
	isLoaded = true;
}

Model::Model(void)
{
	isAnimated = false;
	isLoaded = false;
}
