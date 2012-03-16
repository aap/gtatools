#ifndef WORLD_H
#define WORLD_H
#include <string>
#include <vector>
#include "objects.h"
#include "math.h"

#define END 0
enum iplSections {
	INST = 1,
	CULL,
	ZONE,
	PICK,
	PATH_IPL
};

class Instance
{
public:
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

	/* functions */
	void draw(void);
	void justDraw(void);
	void transform(void);
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
	void draw(void);
	void drawLod(void);
	void drawZones(void);
};

class World
{
private:
	// list of ALL instances
	std::vector<Instance *> instances;

	std::vector<Island> islands;
	uint activeIsland;

	std::vector<Instance *> transpInstances;

	int interior;
	int hour, minute;

	void addInstance(Instance *i);
public:
	int getLod(Instance *);
	Instance *getInstance(uint i);
	void readIpl(std::ifstream &, std::string fileName);
	void readBinIpl(std::ifstream &);
	void readTextIpl(std::ifstream &);
	void associateLods(void);
	void drawIslands(void);
	void addTransparent(Instance *ip, float dist);

	void setInterior(int i);
	int getInterior(void);

	int getHour(void);
	int getMinute(void);
	void setHour(int);
	void setMinute(int);
	World(void);
};

extern World world;

#endif
