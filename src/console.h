#ifndef GTA_CONSOLE_H
#define GTA_CONSOLE_H

#include <GL/glew.h>

class Console
{
private:
	GLuint vbo, bgvbo;
	GLuint tex;
	int width, height;
public:
	void init(void);
	void draw(void);
	void setDimensions(int width, int height);
};

extern Console *console;

#endif

