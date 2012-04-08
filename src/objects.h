#ifndef OBJECTS_H
#define OBJECTS_H
#include "gta.h"
#include "drawable.h"
#include "col.h"

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
public:
	std::string modelName;
	std::string textureName;

	bool isAnimated;
	std::string animationName;

	bool isLoaded;
	Drawable drawable;

	bool BSvisible;
	CollisionModel *col;

	std::vector<quat> boundingSpheres;

	void load(void);
	void drawCol(void);
	void drawBoundingSphere(void);
	Model(void);
};

// ped
class Ped : public Model
{
public:
	std::string defaultPedType;
	std::string behaviour;
	std::string animGroup;
	int carsCanDrive;

	std::string animFile;
	int radio1, radio2;

	std::string voiceArchive;
	std::string voice1, voice2;
};

// car
class Car : public Model
{
public:
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
};

// hier
class Hier : public Model
{
	// just a model
};

// objs, tobj, and anim
class WorldObject : public Model
{
public:
	uint objectCount;
	std::vector<float> drawDistances;
	int flags;

	bool isTimed;
	int timeOn;
	int timeOff;

	bool isVisibleAtTime(int hour);
	int getCorrectAtomic(float d);
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

	void init(int objs);
	void readIde(std::ifstream &in);
	void findAndReadCol(std::string fileName);
	void readCol(std::ifstream &in, int island = -1);
	ObjectList(void);
	~ObjectList(void);
};

extern ObjectList objectList;

#endif
