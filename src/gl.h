#ifndef GL_H
#define GL_H
#include "GL/glew.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gta.h"
#include "pipeline.h"

namespace gl {
	void dumpmat(glm::mat3 &m);
	void dumpmat(glm::mat4 &m);

	void renderScene(void);
	void reshape(int w, int h);
	void keypress(uchar key, int x, int y);
	void keypressSpecial(int key, int x, int y);
	void mouseButton(int button, int state, int x, int y);
	void mouseMotion(int x, int y);
	void init(void);
	void start(int *argc, char *argv[]);

	extern int width;
	extern int height;
	extern Pipeline simplePipe;
	extern Pipeline *currentPipe;

	extern GLuint whiteTex;
	extern bool drawWire;
	extern bool drawTransparent;

	extern glm::mat4 modelMat;
	extern glm::mat4 viewMat;
	extern glm::mat4 projMat;
}

#endif
