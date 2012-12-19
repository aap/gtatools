#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "gta.h"
#include "drawable.h"
#include "animation.h"

using namespace std;

void MixedAnimation::attachAnims(Animation *a, Animation *b, float f)
{
	anim1 = a;
	anim2 = b;
	mixFactor = f;
	if (b == 0)
		endTime = anim1->endTime;
	else
		endTime = anim1->endTime*f + anim2->endTime*(1.0f-f);
}

void MixedAnimation::apply(float t, ::Frame *f)
{
	if (anim1 == 0)
		return;
	if (anim2 == 0) {
		anim1->apply(t, f, true);
		return;
	}
	KeyFrame kf1, kf2;
	anim1->getKeyframe(t, f->name, kf1);
	anim2->getKeyframe(t, f->name, kf2);

	// if the first frame is invalid, swap frame 1 and 2
	if (kf1.type == 0) {
		kf1 = kf2;
		kf2.type = 0;
	}

	if (kf1.type != 0 && kf2.type == 0) {
		glm::quat q;
		q.x = kf1.rot.x;
		q.y = kf1.rot.y;
		q.z = kf1.rot.z;
		q.w = kf1.rot.w;

		if (kf1.type == KRT0 || kf1.type == KRTS) {
			f->pos.x = kf1.pos.x;
			f->pos.y = kf1.pos.y;
			f->pos.z = kf1.pos.z;
		}
		// no scaling yet

		f->modelMat = glm::mat4_cast(q);
		f->modelMat[3][0] = f->pos.x;
		f->modelMat[3][1] = f->pos.y;
		f->modelMat[3][2] = f->pos.z;
	} else if (kf1.type != 0 && kf2.type != 0) {
		quat p = kf1.rot.slerp(kf2.rot, mixFactor);
		p.normalize();
		glm::quat q;
		q.x = p.x;
		q.y = p.y;
		q.z = p.z;
		q.w = p.w;

		if (kf1.type == KRT0 || kf1.type == KRTS) {
			p = kf1.pos*mixFactor + kf2.pos*(1.0f-mixFactor);
			f->pos.x = p.x;
			f->pos.y = p.y;
			f->pos.z = p.z;
		}
		// no scaling yet

		f->modelMat = glm::mat4_cast(q);
		f->modelMat[3][0] = f->pos.x;
		f->modelMat[3][1] = f->pos.y;
		f->modelMat[3][2] = f->pos.z;
	}
	for (size_t i = 0; i < f->children.size(); i++)
		apply(t, f->children[i]);
}

quat MixedAnimation::getPosition(float f, std::string name)
{
	quat pos(0.0, 0.0, 0.0);
/*
	if (anim1 == 0)
		return pos;
	if (anim2 == 0)
		return pos;
*/
	KeyFrame kf1, kf2;
	anim1->getKeyframe(f, name, kf1);
//	anim2->getKeyframe(f, name, kf2);
	if (kf1.type == KRT0 || kf1.type == KRTS) {
		pos.x = kf1.pos.x;
		pos.y = kf1.pos.y;
		pos.z = kf1.pos.z;
	}

/*
	// if the first frame is invalid, swap frame 1 and 2
	if (kf1.type == 0) {
		kf1 = kf2;
		kf2.type = 0;
	}
	if (kf1.type != 0 && kf2.type == 0) {
		if (kf1.type == KRT0 || kf1.type == KRTS) {
			pos.x = kf1.pos.x;
			pos.y = kf1.pos.y;
			pos.z = kf1.pos.z;
		}
	} else if (kf1.type != 0 && kf2.type != 0) {
		if (kf1.type == KRT0 || kf1.type == KRTS) {
			quat p = kf1.pos*mixFactor + kf2.pos*(1.0f-mixFactor);
			pos.x = p.x;
			pos.y = p.y;
			pos.z = p.z;
		}
	}
*/
	return pos;
}

MixedAnimation::MixedAnimation(void)
{
	anim1 = anim2 = 0;
	mixFactor = 0.0f;
	endTime = 0.0f;
}

MixedAnimation::MixedAnimation(Animation *a, Animation *b, float f)
{
	anim1 = a;
	anim2 = b;
	mixFactor = f;
	endTime = anim1->endTime*f + anim2->endTime*(1.0f-f);
}

void AnimPackage::clear(void)
{
	animList.resize(0);
	name = "";
}

void Animation::getKeyframe(float t, string name, KeyFrame &kf)
{
	size_t oi;
	// TODO: probably too slow
	for (oi = 0; oi < objList.size(); oi++)
		if (objList[oi].name == name)
			break;

	if (oi < objList.size())
		objList[oi].interpolate(t, kf);
	else
		kf.type = 0;
}

void Animation::apply(float t, ::Frame *f, bool recurse)
{
	KeyFrame kf;
	getKeyframe(t, f->name, kf);

	if (kf.type != 0) {
		glm::quat q;
		q.x = kf.rot.x;
		q.y = kf.rot.y;
		q.z = kf.rot.z;
		q.w = kf.rot.w;

		if (kf.type == KRT0 || kf.type == KRTS) {
			f->pos.x = kf.pos.x;
			f->pos.y = kf.pos.y;
			f->pos.z = kf.pos.z;
		}
		// no scaling yet

		f->modelMat = glm::mat4_cast(q);
		f->modelMat[3][0] = f->pos.x;
		f->modelMat[3][1] = f->pos.y;
		f->modelMat[3][2] = f->pos.z;
	}

	if (recurse)
		for (size_t i = 0; i < f->children.size(); i++)
			apply(t, f->children[i], recurse);
}

void Animation::clear(void)
{
	objList.resize(0);
	name = "";
}

void AnimObj::interpolate(float t, KeyFrame &key)
{
	if (t >= frmList[frmList.size()-1].timeKey) {
		key = frmList[frmList.size()-1];
		return;
	}

	size_t i;
	for (i = 0; i < frmList.size(); i++)
		if (t < frmList[i].timeKey)
			break;
	KeyFrame &f1 = frmList[i-1];
	KeyFrame &f2 = frmList[i];
	key = f1;

	float r = (f1.timeKey - t)/(f1.timeKey - f2.timeKey);

	key.pos = f1.pos*(1.0f - r) + f2.pos*r;
	key.scale = f1.scale*(1.0f - r) + f2.scale*r;
	key.rot = f1.rot.slerp(f2.rot, r);
}

