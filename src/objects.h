#ifndef OBJECTS_H
#define OBJECTS_H
#include "gta.h"
#include "drawable.h"

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

	void load(void);
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
	WorldObject(void);
};

class ObjectList
{
private:
	Object **objects;
	int objectCount;
public:
	Object *get(int i);
	void add(Object *o);
	int getObjectCount(void);

	void init(int objs);
	void readIde(std::ifstream &in);
	ObjectList(void);
	~ObjectList(void);
};

extern ObjectList objectList;

#endif
