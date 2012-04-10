#ifndef WORLD_H
#define WORLD_H
#include <string>
#include <vector>
#include "objects.h"
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
public:
	uint index;
	int id;
	std::string name;
	int interior;
	quat position;
	quat scale;
	quat rotation;

	bool isLod;
	bool isIslandLod;

	std::vector<int> hires;
	int lod;

	bool isVisible;

	bool wasAdded;

	/* functions */
	bool isCulled(void);
	void addToRenderList(void);
	void transform(void);
	void printInfo(void);
	void setVisible(bool v);
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
	bool pointInZone(quat p);
	bool sphereInZone(quat p, float r);
};

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
	bool isVisible;
	/* functions */
	enum Intersection intersectBox(quat bmin, quat bmax);
	bool isPointInside(quat p);
	void addInstance(Instance *i);
	void setBounds(quat c1, quat c2);
	quat getMinCorner(void);
	quat getMaxCorner(void);
	void addToRenderList(void);
	void draw(void);
};

class World
{
private:
	// list of ALL instances
	std::vector<Instance *> instances;

	std::vector<Island> islands;
	uint activeIsland;

	std::vector<WorldSector> sectors;

	int interior;

	void addInstance(Instance *i);
	void addInstanceToIsland(Instance *i);
	void addInstanceToSectors(Instance *i);
public:
	int getLod(Instance *);
	Instance *getInstance(uint i);
	void readIpl(std::ifstream &, std::string fileName);
	void readBinIpl(std::ifstream &);
	void readTextIpl(std::ifstream &);
	void populateIslands(void);
	void associateLods(void);
	void buildRenderList(void);
	void drawZones(void);

	void setSectorVisible(bool v);

	void initSectors(quat start, quat end, int count);
	void drawSectors(void);

	void setInterior(int i);
	int getInterior(void);

	World(void);
};

extern World world;

#endif
