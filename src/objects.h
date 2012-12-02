#ifndef GTA_OBJECTS_H
#define GTA_OBJECTS_H
#include <string>
#include <vector>
#include <fstream>
#include "gta.h"
#include "math.h"
#include "drawable.h"

class CollisionModel;

#ifndef END
#define END 0
#endif
enum ideSections {
	OBJS = 1,
	TOBJ,
	PATH_IDE,
	TWODFX,
	ANIM,
	TXDP,
	HIER,
	CARS,
	PEDS,
	WEAP
};

// base class for all things defined in IDE files
class Object
{
public:
	uint id;
	uint type;
};

// base class for all objects having a model
class Model : public Object
{
private:
public:
	std::string modelName;
	std::string textureName;

	bool BSvisible;
	bool isAnimated;

	bool isFreshlyLoaded;

	Drawable *drawable;
	CollisionModel *col;
	std::string animationName;

	int refCount;

public:
	bool canDraw(void);
	void load(void);
	void unload(void);
	void drawCol(void);
	void drawBoundingSphere(void);
	void incRefCount(void);
	void decRefCount(void);
	Model(void);
	~Model(void);
};

// ped
class Ped : public Model
{
private:
	std::string defaultPedType;
	std::string behaviour;
	std::string animGroup;
	int carsCanDrive;

	std::string animFile;
	int radio1, radio2;

	std::string voiceArchive;
	std::string voice1, voice2;
public:
	void initFromLine(std::vector<std::string> fields);
};

// car
class Car : public Model
{
	std::string vehicleType;
	std::string handlingId;
	std::string gameName;
	std::string anims;	// vc & sa
	std::string className;
	int frequency;
	int level;
	int compRules;

	// cars
	int wheelModelId;
	float wheelScaleF, wheelScaleR;	// only one value in iii & vc
	int unknown;

	// planes
	int lodId;
public:
	void initFromLine(std::vector<std::string> fields);
};

// hier
class Hier : public Model
{
	// just a model
};

// objs, tobj, and anim
class WorldObject : public Model
{
//private:
public:
	uint objectCount;
	std::vector<float> drawDistances;
	int flags;

	bool isTimed;
	int timeOn;
	int timeOff;

public:
	void initFromLine(std::vector<std::string> fields, int blockType);

	bool isVisibleAtTime(int hour);
	int getCorrectAtomic(float d);
	float getDrawDistance(int atomic);
	WorldObject(void);

	void printInfo(void);
};

class ObjectList
{
private:
	Model **objects;
	int objectCount;
	std::vector<CollisionModel *> cols;
public:
	Model *get(int i);
	Model *get(std::string name);
	void add(Model *o);
	int getObjectCount(void);

	void dumpCols(void);
	void dump(void);
	void associateCols(void);

	void readIde(std::ifstream &in);
	void findAndReadCol(std::string fileName);
	void readCol(std::ifstream &in, int island = -1);
	ObjectList(int objs);
	~ObjectList(void);
};

inline Model *ObjectList::get(int i)
{
	if (i < objectCount && i >= 0)
		return objects[i];
	return 0;
}

inline Model *ObjectList::get(std::string name)
{
	for (int i = 0; i < objectCount; i++)
		if (objects[i] && objects[i]->modelName == name)
			return objects[i];
	return 0;
}

extern ObjectList *objectList;

#endif
