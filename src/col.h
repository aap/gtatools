#ifndef GTA_COL_H
#define GTA_COL_H
#include <string>
#include <vector>
#include "math.h"

struct ColSurface {
	int material;
	int flag;
	int brightness;
	int light;
};

struct ColSphere {
	quat sphere;
	ColSurface surf;
};

struct ColBox {
	quat min;
	quat max;
	ColSurface surf;
};

struct ColFaceGroup {
	quat min;
	quat max;
	int start, end;
};

struct ColFace {
	int a, b, c;
	ColSurface surf;
};

class CollisionModel
{
public:
	std::string name;
	quat boundingSphere;
	quat min;
	quat max;

	std::vector<ColSphere> spheres;
	std::vector<ColBox> boxes;
	std::vector<float> vertices;
	std::vector<ColFaceGroup> faceGroups;
	std::vector<ColFace> faces;

	std::vector<float> shadVertices;
	std::vector<ColFace> shadFaces;

	int island;

	/* functions */
	int read(std::ifstream &col);
};

#endif
