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
	quat underworld(0.5, 0.5, 0.5);
	if (game == GTASA)
		underworld = w->skyBot;
	GLfloat box[] = {
		 1000, -100,  70,
		-1000, -100,  70,
		 1000,  100,  70,
		-1000,  100,  70,
		 1000,  100,  0,
		-1000,  100,  0,
		 1000,  100, -10,
		-1000,  100, -10,
		 1000, -100, -10,
		-1000, -100, -10,
		 1000, -100,  0,
		-1000, -100,  0,
		 1000, -100,  70,
		-1000, -100,  70,

		w->skyTop.x, w->skyTop.y, w->skyTop.z,
		w->skyTop.x, w->skyTop.y, w->skyTop.z,
		w->skyTop.x, w->skyTop.y, w->skyTop.z,
		w->skyTop.x, w->skyTop.y, w->skyTop.z,
		w->skyBot.x, w->skyBot.y, w->skyBot.z,
		w->skyBot.x, w->skyBot.y, w->skyBot.z,
		underworld.x, underworld.y, underworld.z,
		underworld.x, underworld.y, underworld.z,
		underworld.x, underworld.y, underworld.z,
		underworld.x, underworld.y, underworld.z,
		w->skyBot.x, w->skyBot.y, w->skyBot.z,
		w->skyBot.x, w->skyBot.y, w->skyBot.z,
		w->skyTop.x, w->skyTop.y, w->skyTop.z,
		w->skyTop.x, w->skyTop.y, w->skyTop.z
	};

	glm::mat4 mvSave = gl::state.modelView;
	glm::mat3 nrmSave = gl::state.normalMat;

	quat campos = cam.getPosition();
	gl::state.modelView = glm::translate(gl::state.modelView,
			glm::vec3(campos.x, campos.y, campos.z));
	gl::state.modelView = glm::rotate(gl::state.modelView,
	                                  cam.getYaw()/3.1415f*180.0f,
	                                  glm::vec3(0.0f, 0.0f, 1.0f));
	state.updateMatrices();

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

	gl::state.modelView = mvSave;
	gl::state.normalMat = nrmSave;
	gl::state.updateMatrices();
}
