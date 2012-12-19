#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "gta.h"
#include "objects.h"

using namespace std;

void Ped::initFromLine(vector<string> fields)
{
	int i = 0;
	type = PEDS;
	id = atoi(fields[i++].c_str());
	modelName = fields[i++];
	textureName = fields[i++];
	defaultPedType = fields[i++];
	behaviour = fields[i++];
	animGroup = fields[i++];
	stringToLower(modelName);
	stringToLower(textureName);
	stringToLower(defaultPedType);
	stringToLower(behaviour);
	stringToLower(animGroup);
	carsCanDrive = atoi(fields[i++].c_str());

	if (fields.size() >= 10) {	// vc and sa
		animFile = fields[i++];
		stringToLower(animFile);
		radio1 = atoi(fields[i++].c_str());
		radio2 = atoi(fields[i++].c_str());
	}
	if (fields.size() >= 13) {	// sa
		voiceArchive = fields[i++];
		voice1 = fields[i++];
		voice2 = fields[i++];
		stringToLower(voiceArchive);
		stringToLower(voice1);
		stringToLower(voice2);
	}
}

void Ped::reset(void)
{
	frm.position = quat(-138.687,-176.729,16.3685);
	frm.up = quat (0.0, 0.0, 1.0);
	frm.setForward(quat(0.0, 1.0, 0.0));
	state = 0;
	pthread_mutex_init(&(animmut), NULL);
}

void Ped::setAnim(string an)
{
	for (size_t i = 0; i < gl::anpk.animList.size(); i++) {
		if (gl::anpk.animList[i].name == an) {
			drawable->attachAnim(&gl::anpk.animList[i]);
			break;
		}
	}
}

void Ped::draw(void)
{
	glm::mat4 mvSave = gl::state.modelView;
	glm::mat3 nrmSave = gl::state.normalMat;

	gl::state.modelView *= frm.mat;
	quat p = drawable->getPosition();
	gl::state.modelView = glm::translate(gl::state.modelView,
	                                     glm::vec3(0, -p.y, 0));

	gl::state.calculateNormalMat();
	gl::state.updateMatrices();

	if (canDraw()) {
		gl::drawTransparent = false;
		drawable->draw();
		gl::drawTransparent = true;
		drawable->draw();
	}

	gl::state.modelView = mvSave;
	gl::state.normalMat = nrmSave;
	gl::state.updateMatrices();
}

void Ped::incTime(float t)
{
	if (drawable) {
		pthread_mutex_lock(&animmut);
		lastPos = drawable->getPosition();
		bool s = drawable->setTime(drawable->getTime()+t);
		if (s)
			lastPos = lastPos - drawable->getMaxPosition()
			                  + drawable->getMinPosition();
		float dist = (drawable->getPosition() - lastPos).y;
		frm.moveForward(dist);
		pthread_mutex_unlock(&animmut);
	}
}

void Ped::setStateWalking(void)
{
	if (state != 1) {
		pthread_mutex_lock(&animmut);
		state = 1;
//		setAnim("walk_player");
		setAnim("walk_gang1");
		pthread_mutex_unlock(&animmut);
	}
}

void Ped::setStateRunning(void)
{
	if (state != 2) {
		pthread_mutex_lock(&animmut);
		state = 2;
		setAnim("run_player");
		pthread_mutex_unlock(&animmut);
	}
}

void Ped::setStateStop(void)
{
	if (state != 0) {
		pthread_mutex_lock(&animmut);
		state = 0;
		setAnim("idle_stance");
		pthread_mutex_unlock(&animmut);
	}
}

