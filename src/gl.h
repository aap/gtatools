#ifndef GL_H
#define GL_H
#include <pthread.h>

#include "GL/glew.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gta.h"
#include "pipeline.h"

namespace gl {
	void renderScene(void);
	void reshape(int w, int h);
	void keypress(uchar key, int x, int y);
	void keypressSpecial(int key, int x, int y);
	void mouseButton(int button, int state, int x, int y);
	void mouseMotion(int x, int y);
	void init(void);
	void start(int *argc, char *argv[]);

	extern int stencilShift;
	extern int width;
	extern int height;
	extern Pipeline *currentPipe;

	extern GLuint whiteTex;
	extern bool wasTransparent;
	extern bool drawTransparent;
	extern bool drawWire;

	extern Pipeline simplePipe, lambertPipe, gtaPipe;
}

#endif
