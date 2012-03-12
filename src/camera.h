#ifndef CAMERA_H
#define CAMERA_H
#include "math.h"

class Camera
{
public:
	void look(void);
	Camera(void);
	void PanLR(float d);
	void PanUD(float d);
	void turnLR(float phi);
	void turnUD(float phi);
	void moveInOut(float d);
	float distanceTo(quat q);
	void setPitch(float pitch);
	void setYaw(float yaw);
	void setDistance(float d);
	void setTarget(quat q);
	void setFov(float f);
	void setAspectRatio(float r);
	void setNearFar(float n, float f);
	float getPitch(void);
	float getYaw(void);
	float getDistance(void);
	quat getTarget(void);
	quat getPosition(void);
	void changePitch(float pitch);
	void changeYaw(float yaw);
	void changeDistance(float d);
	void changeTarget(quat q);

private:
	void updateCam(void);
	float theta, phi, r;
	float fov, aspectRatio;
	float n, f;
	quat up;
	quat target;
};

extern Camera cam;

#endif
