#include <cstdlib>

#include "gta.h"
#include "enex.h"
#include "gl.h"
#include "pipeline.h"
#include "primitives.h"
#include "renderer.h"
#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

using namespace std;

EnexList *enexList;

void Enex::initFromLine(std::vector<std::string> fields)
{
	int i = 0;
	entry = quat(atof(fields[i].c_str()),
	             atof(fields[i+1].c_str()),
	             atof(fields[i+2].c_str()));
	i += 3;
	entrot = atof(fields[i++].c_str());
	w1 = atof(fields[i++].c_str());
	w2 = atof(fields[i++].c_str());
	i++;	/* skip */
	exit = quat(atof(fields[i].c_str()),
	            atof(fields[i+1].c_str()),
	            atof(fields[i+2].c_str()));
	i += 3;
	interior = atoi(fields[i++].c_str());
	flag = atoi(fields[i++].c_str());

	name = fields[i++];
	sky = atoi(fields[i++].c_str());
	i++;	/* skip */
	timeOn = atoi(fields[i++].c_str());
	timeOff = atoi(fields[i++].c_str());
//	cout << entry << " " << exit << endl;
}

void Enex::draw(void)
{
	glm::mat4 mvSave = gl::state.modelView;
	glm::mat3 nrmSave = gl::state.normalMat;

	gl::state.modelView = glm::translate(gl::state.modelView,
	                                     glm::vec3(entry.x, entry.y, entry.z));

	gl::state.calculateNormalMat();
	gl::state.updateMatrices();

	glVertexAttrib4f(gl::in_Color, 1.0f, 1.0f, 0.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, renderer->whiteTex);
	gl::drawSphere(1.0f, 5, 5);

	gl::state.modelView = mvSave;
	gl::state.normalMat = nrmSave;
	gl::state.updateMatrices();
}

void Enex::enter(void)
{
	cam->setTarget(exit);
}



void EnexList::add(Enex *enex)
{
	enexes.push_back(enex);
}

void EnexList::draw(void)
{
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	for(uint i = 0; i < enexes.size(); i++){
		int stenc = -i;
		glStencilFunc(GL_ALWAYS, (stenc>>gl::stencilShift)&0xFF,-1);
		enexes[i]->draw();
	}
	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
}

void EnexList::enter(int i)
{
	cout << "entering " << i << endl;
	enexes[i]->enter();
}

