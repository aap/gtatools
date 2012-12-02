#ifndef GTA_CAMERA_H
#define GTA_CAMERA_H
#include "math.h"
#include "drawable.h"

class Camera
{
private:
	void updateFrustum(void);
	void updateCam(void);
	float theta, phi, dist;
	float fov, aspectRatio;
	float n, f;
	quat up;
	quat target;
	quat planes[6];
	quat frustumSphere;

	Drawable *aim;
public:
	void panLR(float d);
	void panUD(float d);
	void turnLR(float phi);
	void turnUD(float phi);
	void moveInOut(float d);

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
	float getFov(void);

	quat getPosition(void);

	void lock(Drawable *d);

	void look(void);
	bool isPointInFrustum(quat p);
	bool isSphereInFrustum(quat s);
	bool isBoxInFrustum(quat min, quat max);
	bool isBoxInFrustumSphere(quat min, quat max);
	float distanceTo(quat q);
	Camera(void);
};

extern Camera *cam;

#endif
