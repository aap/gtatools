#include "objects.h"
#include "world.h"

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
		getFields(line, ',', fields);
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
			    (fields.size() == 7 && blockType == TOBJ))
				hasObjectCount = false;

			int i = 0;
			newObj = new WorldObject;
			newObj->type = blockType;
			newObj->id = atoi(fields[i++].c_str());
			newObj->modelName = fields[i++];
			newObj->textureName = fields[i++];
			stringToLower(newObj->modelName);
			stringToLower(newObj->textureName);

			if (blockType == ANIM) {
				newObj->isAnimated = true;
				newObj->animationName = fields[i++];
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
		}
	} while(!in.eof());
}

Object *ObjectList::get(int i)
{
	if (i < objectCount && i >= 0)
		return objects[i];
	return 0;
}

void ObjectList::add(Object *o)
{
	if (o->id >= objectCount || o->id < 0) {
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
	objects = new Object*[objs];
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
	if (drawDistances.size() != objectCount)
		cout << "whaa\n";
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

WorldObject::WorldObject(void)
{
	isTimed = false;
}

/*
 * Model
 */

void Model::load(void)
{
//	cout << modelName << " " << textureName << endl;
	if (drawable.load(modelName+".dff", textureName+".txd") == -1)
		return;
	isLoaded = true;
}

Model::Model(void)
{
	isAnimated = false;
	isLoaded = false;
}
