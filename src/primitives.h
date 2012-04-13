#ifndef GTA_PRIMITIVES_H
#define GTA_PRIMITIVES_H

#include <GL/glew.h>

#include "math.h"

namespace gl {

void drawCube2(quat pos1, quat pos2);
void drawCube(GLfloat size);
void drawSphere(GLfloat radius, GLint slices, GLint stacks);
void drawAxes(GLfloat *mat);

}

#endif
