#include "gta.h"
#include "gl.h"
#include "camera.h"
#include "timecycle.h"
#include "sky.h"

using namespace std;
using namespace gl;

Sky sky;

void Sky::draw(void)
{
	GLuint vbo = 0;

	Weather *w = timeCycle.getCurrentWeatherData();
	GLfloat box[] = {
/*
		 3000, -1000,  500,
		-3000, -1000,  500,
		 3000,  1000,  500,
		-3000,  1000,  500,
		 3000,  1000,  0,
		-3000,  1000,  0,
		 3000,  1000, -100,
		-3000,  1000, -100,
		 3000, -1000, -100,
		-3000, -1000, -100,
		 3000, -1000,  0,
		-3000, -1000,  0,
		 3000, -1000,  500,
		-3000, -1000,  500,
*/

		 1000, -100,  50,
		-1000, -100,  50,
		 1000,  100,  50,
		-1000,  100,  50,
		 1000,  100,  0,
		-1000,  100,  0,
		 1000,  100, -10,
		-1000,  100, -10,
		 1000, -100, -10,
		-1000, -100, -10,
		 1000, -100,  0,
		-1000, -100,  0,
		 1000, -100,  50,
		-1000, -100,  50,

		w->skyTop.x, w->skyTop.y, w->skyTop.z,
		w->skyTop.x, w->skyTop.y, w->skyTop.z,
		w->skyTop.x, w->skyTop.y, w->skyTop.z,
		w->skyTop.x, w->skyTop.y, w->skyTop.z,
		w->skyBot.x, w->skyBot.y, w->skyBot.z,
		w->skyBot.x, w->skyBot.y, w->skyBot.z,
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		w->skyBot.x, w->skyBot.y, w->skyBot.z,
		w->skyBot.x, w->skyBot.y, w->skyBot.z,
		w->skyTop.x, w->skyTop.y, w->skyTop.z,
		w->skyTop.x, w->skyTop.y, w->skyTop.z
	};

	glm::mat4 save = modelMat;

	quat campos = cam.getCamPos();
	gl::modelMat = glm::translate(gl::modelMat,
	                              glm::vec3(campos.x, campos.y, campos.z));
	gl::modelMat = glm::rotate(gl::modelMat, cam.getYaw()/3.1415f*180.0f,
	                           glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 modelView = viewMat * modelMat;
	glUniformMatrix4fv(gl::u_ModelView, 1, GL_FALSE,
	                   glm::value_ptr(modelView));

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(box),
		     box, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(in_Vertex);
	glEnableVertexAttribArray(in_Color);
	glVertexAttribPointer(in_Vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(in_Color, 3, GL_FLOAT, GL_FALSE, 0,
			     (GLvoid *) (14*3*sizeof(GLfloat)));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 14);
	glDisableVertexAttribArray(in_Vertex);
	glDisableVertexAttribArray(in_Color);
	glDeleteBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	modelMat = save;
	modelView = viewMat * modelMat;
	glUniformMatrix4fv(gl::u_ModelView, 1, GL_FALSE,
	                   glm::value_ptr(modelView));
}
