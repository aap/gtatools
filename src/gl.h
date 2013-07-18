#ifndef GTA_GL_H
#define GTA_GL_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "math.h"
#include "pipeline.h"

#include "animation.h"

void handleKeyboardInput(void);
void keypress(int key, int state);
void mouseButton(int button, int state);
void mouseMotion(int x, int y);
void mouseWheel(int pos);
void handleJoystickInput(int joy);

void resize(int w, int h);

namespace gl {
	extern int stencilShift;
	extern int width;
	extern int height;

	extern float alphaVal;
}

class RefFrame {
private:
public:
	void update(void);

	quat forward;
	quat right;
	quat up;
	quat position;
	glm::mat4 mat;

	void move(quat p1, quat p2);
	void moveForward(float d);
	void rotate(float r);
	void setHeading(float r);
	void setForward(quat f);
	void drawSphere(float r);
};

#endif
