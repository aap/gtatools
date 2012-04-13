#ifndef GTA_DRAWABLE_H
#define GTA_DRAWABLE_H

#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <renderware.h>

#include "math.h"
#include "texman.h"
#include "ifp.h"

struct Frame {
	std::string name;
	glm::vec3 pos;
	glm::mat4 modelMat;

	// bones
	int boneId;
	glm::mat4 boneInverseMat;
	glm::mat4 boneMat;

	glm::mat4 ltm;

	Frame *parent;
	std::vector<Frame*> children;

	int geo;
	int index;
};

struct Geometry {
	GLuint vbo;
	GLuint ibo;

	bool isSkinned;
	std::vector<GLfloat> vertices;

	std::vector<GLubyte> vertexColors;

	quat boundingSphere;
};

class Drawable
{
private:
	rw::Clump clump;
	// geometry
	std::vector<Geometry> geoList;
	// frames
	Frame *root;
	std::vector<Frame *> frmList;
	// textures
	TexDictionary *texDict;
	std::vector<int> atomicList;

	Animation anim;
	Frame *animRoot;
	std::vector<uint> boneToFrame;
	float endTime;
	float curTime;

	int currentColorStep;

	void attachTexDict(TexDictionary &t);
	void updateFrames(Frame *r);
	void updateGeometries(void);
	void applyAnim(Frame *f);
	void drawGeometry(int g);

public:
	int load(std::string model, std::string texdict);
	void unload(void);
	void attachClump(rw::Clump &c);	// better use a pointer
	void attachTexDict(rw::TextureDictionary &t);	// better use a pointer
	void attachAnim(Animation &a);
	float getTime(void);
	void setTime(float t);
	void setVertexColors(void);
	void draw(void);
	void drawAtomic(uint a);
	void drawFrame(int fi, bool recurse, bool transform);
	void printFrames(int level, Frame *r);
	void dumpClump(bool detailed);
	std::vector<quat> getBoundingSpheres(void);
};

#endif
