#ifndef WATER_H
#define WATER_H

#include "gta.h"

class Water
{
private:
	GLuint vbo;
	GLuint tex;
	std::vector<GLfloat> vertices;
public:
	void loadWater(std::ifstream &f);
	void loadWaterpro(std::ifstream &f);
	void draw(void);
};

extern Water water;

#endif

