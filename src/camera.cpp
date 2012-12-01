#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "gta.h"
#include "math.h"
#include "pipeline.h"
#include "drawable.h"
#include "camera.h"

Camera cam;

void Camera::look()
{
	updateCam();

	if (aim)
		target = aim->getPosition();

	gl::state.projection = glm::perspective(fov, aspectRatio, n, f);
	gl::state.modelView = glm::mat4(1.0f);
	gl::state.modelView = glm::translate(gl::state.modelView,
	                                     glm::vec3(0.0f, 0.0f, -dist));
	gl::state.modelView = glm::rotate(gl::state.modelView,
	                                  theta/3.1415f*180.0f,
	                                  glm::vec3(1.0f,0.0f,0.0f));
	gl::state.modelView = glm::rotate(gl::state.modelView,
	                                  -phi/3.1415f*180.0f,
	                                  glm::vec3(0.0f,0.0f,1.0f));
	gl::state.modelView = glm::translate(gl::state.modelView,
	                             glm::vec3(-target.x,-target.y, -target.z));
	updateFrustum();
}

void Camera::panLR(float d)
{
	updateCam();
	quat cam(-sin(theta)*sin(phi), sin(theta)*cos(phi), cos(theta));
	quat right = (cam*up - up*cam);
	right.normalize();
	target += right*d;
}

void Camera::panUD(float d)
{
	updateCam();
	quat cam(-sin(theta)*sin(phi), sin(theta)*cos(phi), cos(theta));
	quat right = (cam*up - up*cam);
	quat up_local = (right*cam - cam*right);
	up_local.normalize();
	target += up_local*d;
}

void Camera::turnLR(float ang)
{
	updateCam();
	// calculate cam position, subtract new cam vector
	quat pos(-sin(theta)*sin(phi), sin(theta)*cos(phi), cos(theta));
	pos = pos * dist + target;

	phi += ang;
	quat newcam(-sin(theta)*sin(phi), sin(theta)*cos(phi), cos(theta));

	target =  pos - newcam * dist;
}

void Camera::turnUD(float ang)
{
	updateCam();
	// calculate cam position, subtract new cam vector
	quat cam(-sin(theta)*sin(phi), sin(theta)*cos(phi), cos(theta));
	quat pos = cam * dist + target;

	theta += ang;
	quat newcam(-sin(theta)*sin(phi), sin(theta)*cos(phi), cos(theta));

	target =  pos - newcam * dist;
}

void Camera::moveInOut(float d)
{
	quat cam(-sin(theta)*sin(phi), sin(theta)*cos(phi), cos(theta));

	target -= cam*d;
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

	quat cam(-sin(theta)*sin(phi), sin(theta)*cos(phi), cos(theta));
	quat look = -cam;
	cam = target + cam*dist;
	frustumSphere = cam + look*(n + len*0.5f);
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

void Camera::updateCam()
{
	if (dist < 0.0f)
		dist = 0.0f;

	if (phi < 0.0f)
		phi += 2.0f*PI;
	else if (phi > 2.0f*PI)
		phi -= 2.0f*PI;

	if (theta < 0.0f)
		theta += 2.0f*PI;
	else if (theta > 2.0f*PI)
		theta -= 2.0f*PI;

/*
	if ((0 <= theta && theta <= PI/2.0f) ||
	    (3.0f/2.0f*PI <= theta && theta <= 2.0f*PI))
		up.z = 1.0f;
	else
		up.z = -1.0f;
*/
}

float Camera::distanceTo(quat q)
{
	quat cam(-sin(theta)*sin(phi), sin(theta)*cos(phi), cos(theta));
	cam *= dist;
	cam += target;

	cam -= q;
	return cam.norm();
}

void Camera::setPitch(float pitch)
{
	theta = pitch;
}

void Camera::setYaw(float yaw)
{
	phi = yaw;
}

void Camera::setDistance(float d)
{
	dist = d;
}

void Camera::setTarget(quat q)
{
	target = q;
}

float Camera::getPitch(void)
{
	return theta;
}

float Camera::getYaw(void)
{
	return phi;
}

float Camera::getDistance(void)
{
	return dist;
}

quat Camera::getTarget(void)
{
	return target;
}

float Camera::getFov(void)
{
	return fov;
}

quat Camera::getPosition(void)
{
	updateCam();
	quat pos(-sin(theta)*sin(phi), sin(theta)*cos(phi), cos(theta));
	return pos * dist + target;
}

void Camera::lock(Drawable *d)
{
	aim = d;
}

void Camera::setFov(float f)
{
	fov = f;
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

Camera::Camera()
{
	theta = phi = 0.0f;
	dist = 1.0f;
	up = quat(0.0f, 0.0f, 1.0f);
	target = quat(0.0f, 0.0f, 0.0f);
	fov = 70.0f;
	aspectRatio = 1.0f;
	n = 0.1f;
	f = 10000.0f;
	aim = 0;
}

