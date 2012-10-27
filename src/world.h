#ifndef GTA_WORLD_H
#define GTA_WORLD_H

#include <string>
#include <vector>
#include <fstream>
#include "gta.h"
#include "math.h"

#define END 0
enum IplSections {
	INST = 1,
	CULL,
	ZONE,
	PICK,
	PATH_IPL
};

enum Intersection {
	OUTSIDE = 0,
	INTERSECT,
	INSIDE
};

class Instance
{
private:
	bool isActive;
	float fadingFactor;
public:
	uint index;
	int id;
	std::string name;
	int interior;
	quat position;
	quat scale;
	quat rotation;

	int lod;
	std::vector<int> hires;

	bool wasAdded;

	bool isLod;
	bool isIslandLod;

	bool isHidden;


	/* functions */
	void addToRenderList(void);
	void addLodToRenderList(void);
	bool isCulled(void);
	void transform(void);

	void releaseIfInactive(void);

	void initFromLine(std::vector<std::string> fields);
	void initFromBinEntry(char *data);

	void setFading(float fade);
	float getFading(void);
	Instance(void);

	void printInfo(void);
};

class Zone
{
public:
	std::string name;
	int type;
	quat corner1;
	quat corner2;
	int islandNum;

	// sa
	std::string text;

	/* functions */
	void initFromLine(std::vector<std::string> fields);

	bool pointInZone(quat p);
	bool sphereInZone(quat p, float r);
};

// not used at the moment
class Island
{
public:
	std::vector<Instance *> islandLods;
	std::vector<Instance *> instances;

	std::vector<Zone *> zones;
	std::vector<Zone *> islandZones;

	/* functions */
	bool pointInIsland(quat p);
	bool sphereInIsland(quat p, float r);
	void addToRenderList(void);
	void addLodToRenderList(void);
	void drawZones(void);
};

class WorldSector
{
private:
	quat min;
	quat max;
	std::vector<Instance *> instances;

public:
	/* functions */
	void addInstance(Instance *i);

	void addToRenderList(void);
	void draw(void);

	enum Intersection intersectBox(quat bmin, quat bmax);
	bool isPointInside(quat p);

	void setBounds(quat c1, quat c2);
	quat getMinCorner(void);
	quat getMaxCorner(void);
};

class World
{
private:
	// list of all instances
	std::vector<Instance *> instances;

	std::vector<Island> islands;
	uint activeIsland;

	std::vector<WorldSector> sectors;

	int interior;


	void readBinIpl(std::ifstream& f);
	void readTextIpl(std::ifstream& f);
	void addInstance(Instance *i);
	void addInstanceToIsland(Instance *i);
	void addInstanceToSectors(Instance *i);
	int getLod(Instance *i);
public:
	void initSectors(quat start, quat end, int count);
	void readIpl(std::ifstream& f, std::string fileName);
	void populateIslands(void);
	void associateLods(void);

	void drawSectors(void);
	void drawZones(void);
	void buildRenderList(void);

	void cleanUp(void);

	Instance *getInstance(uint i);
	void setInterior(int i);
	int getInterior(void);

	World(void);
};

extern World world;

#endif
