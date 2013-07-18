#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "gta.h"
#include "gl.h"
#include "math.h"
#include "primitives.h"
#include "pipeline.h"
#include "drawable.h"
#include "camera.h"

Camera *cam;

void Camera::look(void)
{
	if (aim)
		target = aim->position;

	gl::state.projection = glm::perspective(fov, aspectRatio, n, f);

	gl::state.modelView =
		glm::lookAt(glm::vec3(position.x, position.y, position.z),
		            glm::vec3(target.x, target.y, target.z),
		            glm::vec3(up.x, up.y, up.z));
	updateFrustum();
}

void Camera::setPosition(quat q)
{
	position = q;
}

quat Camera::getPosition(void)
{
	return position;
}

void Camera::setTarget(quat q)
{
	target = q;
}

quat Camera::getTarget(void)
{
	return target;
}

float Camera::getHeading(void)
{
	quat dir = target - position;
	return atan2(dir.y, dir.x);
}

void Camera::turn(float yaw, float pitch)
{
	quat dir = target - position;
	quat r(cos(yaw), up*sin(yaw));
	dir = r*dir*r.conjugate();

	quat right = (dir^up).normalize();
	r = quat(cos(pitch), right*sin(pitch));
	dir = r*dir*r.conjugate();

	target = position + dir;
}

void Camera::orbit(float yaw, float pitch)
{
	quat dir = position - target;
	quat r(cos(yaw), up*sin(yaw));
	dir = r*dir*r.conjugate();

	quat right = (dir^up).normalize();
	r = quat(cos(pitch), right*sin(pitch));
	dir = r*dir*r.conjugate();

	position = target + dir;
}

void Camera::dolly(float dist)
{
	quat dir = (target - position).normalize()*dist;
	position += dir;
	target += dir;
}

void Camera::zoom(float dist)
{
	quat dir = (target - position).normalize()*dist;
	position += dir;
}

void Camera::drawTarget(void)
{
	glm::mat4 m = glm::translate(glm::mat4(1.0f),
		glm::vec3(target.x, target.y, target.z));
	gl::drawAxes(glm::value_ptr(m));
}

void Camera::updateFrustum(void)
{
	// frustum planes
	glm::mat4 m = gl::state.projection * gl::state.modelView;

	float l;

	planes[0] = quat(m[0][2] + m[0][3],
	                 m[1][2] + m[1][3],
	                 m[2][2] + m[2][3]);
	l = planes[0].norm();
	planes[0].w = m[3][2] + m[3][3];
	planes[0] /= l;

	planes[1] = quat(-m[0][2] + m[0][3],
	                 -m[1][2] + m[1][3],
	                 -m[2][2] + m[2][3]);
	l = planes[1].norm();
	planes[1].w = -m[3][2] + m[3][3];
	planes[1] /= l;

	planes[2] = quat(m[0][1] + m[0][3],
	                 m[1][1] + m[1][3],
	                 m[2][1] + m[2][3]);
	l = planes[2].norm();
	planes[2].w = m[3][1] + m[3][3];
	planes[2] /= l;

	planes[3] = quat(-m[0][1] + m[0][3],
	                 -m[1][1] + m[1][3],
	                 -m[2][1] + m[2][3]);
	l = planes[3].norm();
	planes[3].w = -m[3][1] + m[3][3];
	planes[3] /= l;

	planes[4] = quat(m[0][0] + m[0][3],
	                 m[1][0] + m[1][3],
	                 m[2][0] + m[2][3]);
	l = planes[4].norm();
	planes[4].w = m[3][0] + m[3][3];
	planes[4] /= l;

	planes[5] = quat(-m[0][0] + m[0][3],
	                 -m[1][0] + m[1][3],
	                 -m[2][0] + m[2][3]);
	l = planes[5].norm();
	planes[5].w = -m[3][0] + m[3][3];
	planes[5] /= l;

	// frustum sphere
	float len = f - n;
	float height = f * tan(fov*0.5f);
	float width = aspectRatio*height;
	quat P(0.0f, 0.0f, n + len*0.5f);
	quat Q(width, height, f);

	frustumSphere = position + (position-target).norm()*(n + len*0.5f);
	frustumSphere.w = (P - Q).norm();
}

bool Camera::isPointInFrustum(quat p)
{
	for (uint i = 0; i < 6; i++) {
		quat n = planes[i];
		n.w = 0;
		float d = planes[i].w;
		if (p.dot(n) + d <= 0)
			return false;
	}
	return true;
}

bool Camera::isSphereInFrustum(quat s)
{
	float r = s.w;
	s.w = 0;
	for (uint i = 0; i < 6; i++) {
		quat n = planes[i];
		n.w = 0;
		float d = planes[i].w;
		if (s.dot(n) + d <= -r)
			return false;
	}
	return true;
}

bool Camera::isBoxInFrustum(quat min, quat max)
{
	if (!isBoxInFrustumSphere(min, max))
		return false;

	quat corners[8];
	corners[0] = quat(min.x, min.y, min.z);
	corners[1] = quat(min.x, min.y, max.z);
	corners[2] = quat(min.x, max.y, min.z);
	corners[3] = quat(max.x, min.y, min.z);
	corners[4] = quat(max.x, max.y, max.z);
	corners[5] = quat(max.x, max.y, min.z);
	corners[6] = quat(max.x, min.y, max.z);
	corners[7] = quat(min.x, max.y, max.z);

	for (int j = 0; j < 6; j++) {
		quat n = planes[j];
		n.w = 0;
		float d = planes[j].w;

		int nBehind = 0;
		for (int i = 0; i < 8; i++)
			if (corners[i].dot(n) + d <= 0)
				nBehind++;
		if (nBehind == 8)
			return false;
		if (nBehind != 0)
			return true;
	}
	return true;
}

bool Camera::isBoxInFrustumSphere(quat min, quat max)
{
	return !isSphereOutsideBox(frustumSphere, min, max);
}

float Camera::sqDistanceTo(quat q)
{
	return (position - q).normsq();
}

float Camera::distanceTo(quat q)
{
	return (position - q).norm();
}

void Camera::setFov(float f)
{
	fov = f;
}

float Camera::getFov(void)
{
	return fov;
}

void Camera::setAspectRatio(float r)
{
	aspectRatio = r;
}

void Camera::setNearFar(float n, float f)
{
	this->n = n;
	this->f = f;
}

void Camera::lock(RefFrame *f)
{
	aim = f;
}

Camera::Camera()
{
	doTarget = false;
	position = quat(50.0f, 0.0f, 0.0f);
	target = quat(0.0f, 0.0f, 0.0f);
	up = quat(0.0f, 0.0f, 1.0f);
	fov = 70.0f;
	aspectRatio = 1.0f;
	n = 0.1f;
	f = 10000.0f;
	aim = 0;
}

