#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gta.h"
#include "gl.h"
#include "camera.h"

Camera cam;

void Camera::look()
{
	updateCam();

	gl::projMat = glm::perspective(fov, aspectRatio, n, f);
	gl::viewMat = glm::mat4(1.0f);

	gl::viewMat = glm::translate(gl::viewMat, glm::vec3(0.0f, 0.0f, -r));
	gl::viewMat = glm::rotate(gl::viewMat, theta/3.1415f*180.0f,
	                          glm::vec3(1.0f,0.0f,0.0f));
	gl::viewMat = glm::rotate(gl::viewMat, -phi/3.1415f*180.0f,
	                          glm::vec3(0.0f,0.0f,1.0f));
	gl::viewMat = glm::translate(gl::viewMat,
	                             glm::vec3(-target.x,-target.y, -target.z));
	updateFrustum();
}

void Camera::PanLR(float d)
{
	updateCam();
	quat cam(-sin(theta)*sin(phi), sin(theta)*cos(phi), cos(theta));
	quat right = (cam*up - up*cam);
	right.normalize();
	target += right*d;
}

void Camera::PanUD(float d)
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
	pos = pos * r + target;

	phi += ang;
	quat newcam(-sin(theta)*sin(phi), sin(theta)*cos(phi), cos(theta));

	target =  pos - newcam * r;
}

void Camera::turnUD(float ang)
{
	updateCam();
	// calculate cam position, subtract new cam vector
	quat cam(-sin(theta)*sin(phi), sin(theta)*cos(phi), cos(theta));
	quat pos = cam * r + target;

	theta += ang;
	quat newcam(-sin(theta)*sin(phi), sin(theta)*cos(phi), cos(theta));

	target =  pos - newcam * r;
}

void Camera::moveInOut(float d)
{
	quat cam(-sin(theta)*sin(phi), sin(theta)*cos(phi), cos(theta));

	target -= cam*d;
}

void Camera::updateFrustum(void)
{
	glm::mat4 m = gl::projMat * gl::viewMat;

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
}

bool Camera::isPointInFrustum(quat p)
{
	for (uint i = 0; i < 6; i++) {
		quat n = planes[i];
		n.w = 0;
		float d = planes[i].w;
		if (p.dot(n) + d <= -r)
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

void Camera::updateCam()
{
	if (r < 0.0f)
		r = 0.0f;

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
	cam *= r;
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
	r = d;
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
	return r;
}

quat Camera::getTarget(void)
{
	return target;
}

quat Camera::getPosition(void)
{
	updateCam();
	quat pos(-sin(theta)*sin(phi), sin(theta)*cos(phi), cos(theta));
	return pos * r + target;
}

void Camera::changePitch(float pitch)
{
	theta += pitch;
}

void Camera::changeYaw(float yaw)
{
	phi += yaw;
}

void Camera::changeDistance(float d)
{
	r += d;
}

void Camera::changeTarget(quat q)
{
	target += q;
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

quat Camera::getCamPos(void)
{
	quat cam(-sin(theta)*sin(phi), sin(theta)*cos(phi), cos(theta));
	cam *= r;
	cam += target;
	return cam;
}

Camera::Camera()
{
	theta = phi = 0.0f;
	r = 1.0f;
	up = quat(0.0f, 0.0f, 1.0f);
	target = quat(0.0f, 0.0f, 0.0f);
	fov = 70.0f;
	aspectRatio = 1.0f;
	n = 0.1f;
	f = 10000.0f;
}

