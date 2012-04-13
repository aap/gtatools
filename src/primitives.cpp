#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "math.h"
#include "primitives.h"
#include "pipeline.h"

using namespace std;

namespace gl {

void drawSphere(GLfloat r, GLint slices, GLint stacks)
{
	GLuint vbo_sphere;
	vector<GLfloat> sphere;

	for (GLfloat s = PI/2.0f; s > -PI/2.0f; s -= PI/stacks) {
		for (GLfloat t = 0.0f; t < 2*PI; t += 2*PI/slices) {
			sphere.push_back(r*cos(s-PI/stacks)*cos(t));
			sphere.push_back(r*cos(s-PI/stacks)*sin(t));
			sphere.push_back(r*sin(s-PI/stacks));

			sphere.push_back(r*cos(s-PI/stacks)*cos(t+2*PI/slices));
			sphere.push_back(r*cos(s-PI/stacks)*sin(t+2*PI/slices));
			sphere.push_back(r*sin(s-PI/stacks));

			sphere.push_back(r*cos(s)*cos(t+2*PI/slices));
			sphere.push_back(r*cos(s)*sin(t+2*PI/slices));
			sphere.push_back(r*sin(s));

			sphere.push_back(r*cos(s)*cos(t));
			sphere.push_back(r*cos(s)*sin(t));
			sphere.push_back(r*sin(s));
		}
	}

	glm::vec3 n;
	for (GLfloat s = PI/2.0f; s > -PI/2.0f; s -= PI/stacks) {
		for (GLfloat t = 0.0f; t < 2*PI; t += 2*PI/slices) {
			n.x = r*cos(s-PI/stacks)*cos(t);
			n.y = r*cos(s-PI/stacks)*sin(t);
			n.z = r*sin(s-PI/stacks);
			n = glm::normalize(n);
			sphere.push_back(n.x);
			sphere.push_back(n.y);
			sphere.push_back(n.z);

			n.x = r*cos(s-PI/stacks)*cos(t+2*PI/slices);
			n.y = r*cos(s-PI/stacks)*sin(t+2*PI/slices);
			n.z = r*sin(s-PI/stacks);
			n = glm::normalize(n);
			sphere.push_back(n.x);
			sphere.push_back(n.y);
			sphere.push_back(n.z);

			n.x = r*cos(s)*cos(t+2*PI/slices);
			n.y = r*cos(s)*sin(t+2*PI/slices);
			n.z = r*sin(s);
			n = glm::normalize(n);
			sphere.push_back(n.x);
			sphere.push_back(n.y);
			sphere.push_back(n.z);

			n.x = r*cos(s)*cos(t);
			n.y = r*cos(s)*sin(t);
			n.z = r*sin(s);
			n = glm::normalize(n);
			sphere.push_back(n.x);
			sphere.push_back(n.y);
			sphere.push_back(n.z);
		}
	}

	glGenBuffers(1, &vbo_sphere);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere);
	glBufferData(GL_ARRAY_BUFFER, sphere.size()*sizeof(GLfloat),
	             &sphere[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(in_Vertex);
	glEnableVertexAttribArray(in_Normal);
	glVertexAttribPointer(in_Vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(in_Normal, 3, GL_FLOAT, GL_FALSE, 0,
	                      (GLvoid *) (sphere.size()/2*sizeof(GLfloat)));
	glDrawArrays(GL_QUADS, 0, sphere.size()/3/2);
	glDisableVertexAttribArray(in_Vertex);
	glDisableVertexAttribArray(in_Normal);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &vbo_sphere);
}

void drawCube2(quat pos1, quat pos2)
{
	GLuint vbo_cube;
	GLfloat cube[] = {
		// verts
		// bottom
		pos1.x, pos1.y, pos1.z,
		pos1.x, pos2.y, pos1.z,
		pos2.x, pos2.y, pos1.z,
		pos2.x, pos1.y, pos1.z,
		// top
		pos2.x, pos1.y, pos2.z,
		pos2.x, pos2.y, pos2.z,
		pos1.x, pos2.y, pos2.z,
		pos1.x, pos1.y, pos2.z,
		// left
		pos1.x, pos1.y, pos1.z,
		pos1.x, pos1.y, pos2.z,
		pos1.x, pos2.y, pos2.z,
		pos1.x, pos2.y, pos1.z,
		// right
		pos2.x, pos2.y, pos1.z,
		pos2.x, pos2.y, pos2.z,
		pos2.x, pos1.y, pos2.z,
		pos2.x, pos1.y, pos1.z,
		// front
		pos1.x, pos1.y, pos1.z,
		pos2.x, pos1.y, pos1.z,
		pos2.x, pos1.y, pos2.z,
		pos1.x, pos1.y, pos2.z,
		// back
		pos1.x, pos2.y, pos2.z,
		pos2.x, pos2.y, pos2.z,
		pos2.x, pos2.y, pos1.z,
		pos1.x, pos2.y, pos1.z,

		// normals
		// bottom
		 0.0f,  0.0f, -1.0f,
		 0.0f,  0.0f, -1.0f,
		 0.0f,  0.0f, -1.0f,
		 0.0f,  0.0f, -1.0f,
		// top
		 0.0f,  0.0f,  1.0f,
		 0.0f,  0.0f,  1.0f,
		 0.0f,  0.0f,  1.0f,
		 0.0f,  0.0f,  1.0f,
		// left
		-1.0f,  0.0f,  0.0f,
		-1.0f,  0.0f,  0.0f,
		-1.0f,  0.0f,  0.0f,
		-1.0f,  0.0f,  0.0f,
		// right
		 1.0f,  0.0f,  0.0f,
		 1.0f,  0.0f,  0.0f,
		 1.0f,  0.0f,  0.0f,
		 1.0f,  0.0f,  0.0f,
		// front
		 0.0f, -1.0f,  0.0f,
		 0.0f, -1.0f,  0.0f,
		 0.0f, -1.0f,  0.0f,
		 0.0f, -1.0f,  0.0f,
		// back
		 0.0f,  1.0f,  0.0f,
		 0.0f,  1.0f,  0.0f,
		 0.0f,  1.0f,  0.0f,
		 0.0f,  1.0f,  0.0f,
	};
	glGenBuffers(1, &vbo_cube);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
	glEnableVertexAttribArray(in_Vertex);
//	glEnableVertexAttribArray(in_Normal);
	glVertexAttribPointer(in_Vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(in_Normal, 3, GL_FLOAT, GL_FALSE, 0,
	                     (GLvoid *) (6*4*3*sizeof(GLfloat)));
	glDrawArrays(GL_QUADS, 0, 6*4);
	glDisableVertexAttribArray(in_Vertex);
	glDisableVertexAttribArray(in_Normal);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &vbo_cube);
}

void drawCube(GLfloat size)
{
	GLfloat s = size/2.0f;
	GLuint vbo_cube;
	GLfloat cube[] = {
		// verts
		// bottom
		-s, -s, -s,
		-s,  s, -s,
		 s,  s, -s,
		 s, -s, -s,
		// top
		 s, -s,  s,
		 s,  s,  s,
		-s,  s,  s,
		-s, -s,  s,
		// left
		-s, -s, -s,
		-s, -s,  s,
		-s,  s,  s,
		-s,  s, -s,
		// right
		 s,  s, -s,
		 s,  s,  s,
		 s, -s,  s,
		 s, -s, -s,
		// front
		-s, -s, -s,
		 s, -s, -s,
		 s, -s,  s,
		-s, -s,  s,
		// back
		-s,  s,  s,
		 s,  s,  s,
		 s,  s, -s,
		-s,  s, -s,

		// normals
		// bottom
		 0.0f,  0.0f, -1.0f,
		 0.0f,  0.0f, -1.0f,
		 0.0f,  0.0f, -1.0f,
		 0.0f,  0.0f, -1.0f,
		// top
		 0.0f,  0.0f,  1.0f,
		 0.0f,  0.0f,  1.0f,
		 0.0f,  0.0f,  1.0f,
		 0.0f,  0.0f,  1.0f,
		// left
		-1.0f,  0.0f,  0.0f,
		-1.0f,  0.0f,  0.0f,
		-1.0f,  0.0f,  0.0f,
		-1.0f,  0.0f,  0.0f,
		// right
		 1.0f,  0.0f,  0.0f,
		 1.0f,  0.0f,  0.0f,
		 1.0f,  0.0f,  0.0f,
		 1.0f,  0.0f,  0.0f,
		// front
		 0.0f, -1.0f,  0.0f,
		 0.0f, -1.0f,  0.0f,
		 0.0f, -1.0f,  0.0f,
		 0.0f, -1.0f,  0.0f,
		// back
		 0.0f,  1.0f,  0.0f,
		 0.0f,  1.0f,  0.0f,
		 0.0f,  1.0f,  0.0f,
		 0.0f,  1.0f,  0.0f,
	};
	glGenBuffers(1, &vbo_cube);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
	glEnableVertexAttribArray(in_Vertex);
	glEnableVertexAttribArray(in_Normal);
	glVertexAttribPointer(in_Vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(in_Normal, 3, GL_FLOAT, GL_FALSE, 0,
	                     (GLvoid *) (6*4*3*sizeof(GLfloat)));
	glDrawArrays(GL_QUADS, 0, 6*4);
	glDisableVertexAttribArray(in_Vertex);
	glDisableVertexAttribArray(in_Normal);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &vbo_cube);
}

void drawAxes(GLfloat *mat)
{
	GLuint vbo_axes;
	GLfloat axes[3*2*3 + 3*2*4];

	axes[0] = axes[6] = axes[12] = mat[12];
	axes[1] = axes[7] = axes[13] = mat[13];
	axes[2] = axes[8] = axes[14] = mat[14];
	axes[3] = mat[0] + mat[12];
	axes[4] = mat[1] + mat[13];
	axes[5] = mat[2] + mat[14];
	axes[9] = mat[4] + mat[12];
	axes[10] = mat[5] + mat[13];
	axes[11] = mat[6] + mat[14];
	axes[15] = mat[8] + mat[12];
	axes[16] = mat[9] + mat[13];
	axes[17] = mat[10] + mat[14];

	axes[18] = 1.0f;
	axes[19] = 0.0f;
	axes[20] = 0.0f;
	axes[21] = 1.0f;
	axes[22] = 1.0f;
	axes[23] = 0.0f;
	axes[24] = 0.0f;
	axes[25] = 1.0f;

	axes[26] = 0.0f;
	axes[27] = 1.0f;
	axes[28] = 0.0f;
	axes[29] = 1.0f;
	axes[30] = 0.0f;
	axes[31] = 1.0f;
	axes[32] = 0.0f;
	axes[33] = 1.0f;

	axes[34] = 0.0f;
	axes[35] = 0.0f;
	axes[36] = 1.0f;
	axes[37] = 1.0f;
	axes[38] = 0.0f;
	axes[39] = 0.0f;
	axes[40] = 1.0f;
	axes[41] = 1.0f;

	glGenBuffers(1, &vbo_axes);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_axes);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);
	glEnableVertexAttribArray(in_Vertex);
	glEnableVertexAttribArray(in_Color);
	glVertexAttribPointer(in_Vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(in_Color, 4, GL_FLOAT, GL_FALSE, 0,
	                      (GLvoid*) (3*2*3*sizeof(GLfloat)));
	glDrawArrays(GL_LINES, 0, 3*2);
	glDisableVertexAttribArray(in_Vertex);
	glDisableVertexAttribArray(in_Color);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &vbo_axes);
}

}
