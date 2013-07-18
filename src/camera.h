#ifndef GTA_CAMERA_H
#define GTA_CAMERA_H
#include "math.h"

class RefFrame;

/*
 * Three types:
 * 1. Position + Target
 * 2. First Person Cam
 * 3. Orbit Cam
 */
class Camera
{
private:
	void updateFrustum(void);
	void updateCam(void);

	quat position;
	quat target;
	quat up;

	float fov, aspectRatio;
	float n, f;
	quat planes[6];
	quat frustumSphere;

	RefFrame *aim;
public:
	bool doTarget;

	void setPosition(quat q);
	quat getPosition(void);
	void setTarget(quat q);
	quat getTarget(void);
	float getHeading(void);

	void turn(float yaw, float pitch);
	void orbit(float yaw, float pitch);
	void dolly(float dist);
	void zoom(float dist);

	void setFov(float f);
	float getFov(void);
	void setAspectRatio(float r);
	void setNearFar(float n, float f);

	void lock(RefFrame *f);

	void look(void);
	void drawTarget(void);
	bool isPointInFrustum(quat p);
	bool isSphereInFrustum(quat s);
	bool isBoxInFrustum(quat min, quat max);
	bool isBoxInFrustumSphere(quat min, quat max);
	float distanceTo(quat q);
	float sqDistanceTo(quat q);
	Camera(void);
};

extern Camera *cam;

#endif
