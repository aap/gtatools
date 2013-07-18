#ifndef GTA_WATER_H
#define GTA_WATER_H

#include <vector>
#include <fstream>
#include <GL/glew.h>

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

extern Water *water;

#endif

