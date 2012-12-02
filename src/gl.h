#ifndef GTA_GL_H
#define GTA_GL_H

#include <GL/glew.h>
#include "gta.h"
#include "pipeline.h"
#include "animation.h"

namespace gl {
	void handleKeyboardInput(void);
	void keypress(int key, int state);
	void mouseButton(int button, int state);
	void mouseMotion(int x, int y);
	void handleJoystickInput(int joy);

	void resize(int w, int h);
	int initGl(void);
	void *opengl(void *args);

	extern int stencilShift;
	extern int width;
	extern int height;
	extern Pipeline *currentPipe;

	extern GLuint whiteTex;
	extern bool wasTransparent;
	extern bool drawTransparent;
	extern bool drawWire;

	extern Pipeline *simplePipe, *lambertPipe, *gtaPipe;

	/* XXX */
	extern AnimPackage anpk;
}

#endif
