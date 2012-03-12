#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include <GL/glew.h>

namespace gl {

void drawCube2(GLfloat *pos1, GLfloat *pos2);
void drawCube(GLfloat size);
void drawSphere(GLfloat radius, GLint slices, GLint stacks);
void drawAxes(GLfloat *mat);

}

#endif
