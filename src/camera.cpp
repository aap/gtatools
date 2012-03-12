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

Camera::Camera()
{
	theta = phi = 0.0f;
	r = 1.0f;
	up = quat(0.0f, 0.0f, 1.0f);
	target = quat(0.0f, 0.0f, 0.0f);
	fov = 70.0f;
	aspectRatio = 1.0f;
	n = 0.1f;
	f = 100000.0f;
}

