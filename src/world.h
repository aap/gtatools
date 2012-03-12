#ifndef WORLD_H
#define WORLD_H
#include <string>
#include <vector>
#include "objects.h"

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
	float position[3];
	float scale[3];
	float rotation[4];

	bool isLod;
	bool isIslandLod;

	std::vector<int> hires;
	int lod;

	/* functions */
	void transform(void);
};

class Zone
{
public:
	std::string name;
	int type;
	float corner1[3];
	float corner2[3];
	int islandNum;

	// sa
	std::string text;

	/* functions */
	bool pointInZone(float *p);
};

class Island
{
public:
	std::vector<int> lodIds;
	std::vector<Instance *> islandLods;
	std::vector<Instance *> instances;

	std::vector<Zone *> zones;
	std::vector<Zone *> islandZones;

	/* functions */
	bool pointInIsland(float *p);
	void draw(void);
	void drawLod(void);
	void drawZones(void);
};

class World
{
private:
	std::vector<Instance *> instances;

	std::vector<Island> islands;

	uint activeIsland;

public:
	void drawIslands(void);

	int getLod(Instance *);
	void readIpl(std::ifstream &, std::string fileName);
	void readBinIpl(std::ifstream &);
	void readTextIpl(std::ifstream &);
	void associateLods(void);
	void dump(void);

};

extern World world;

#endif
