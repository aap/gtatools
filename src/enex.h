#ifndef GTA_ENEX_H
#define GTA_ENEX_H

#include <string>
#include <vector>
#include "math.h"

class Enex
{
private:
	quat entry;
	float entrot;
	float w1, w2;
	quat exit;
	float exitrot;
	int interior;
	int flag;
	std::string name;
	int sky;
	int timeOn, timeOff;

public:
	void initFromLine(std::vector<std::string> fields);
	void enter(void);
	void draw(void);
};

class EnexList
{
private:
	std::vector<Enex *> enexes;
public:
	void add(Enex *enex);
	void draw(void);
	void enter(int i);
};

extern EnexList *enexList;

#endif
