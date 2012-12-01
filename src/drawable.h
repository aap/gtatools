#ifndef GTA_DRAWABLE_H
#define GTA_DRAWABLE_H

#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <renderware.h>

#include "math.h"
#include "texman.h"
#include "animation.h"

struct Frame {
	std::string name;
	glm::vec3 pos;
	glm::mat4 modelMat;

	glm::vec3 defPos;
	glm::mat4 defMat;

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

	bool dirty;

	bool isSkinned;
	std::vector<GLfloat> vertices;

	std::vector<GLubyte> vertexColors;
};

class Drawable
{
private:
	// geometry
	rw::Clump *clump;
	std::vector<Geometry> geoList;
	// frames
	Frame *root;
	std::vector<Frame *> frmList;
	// textures
	TexDictionary *texDict;
	std::vector<int> atomicList;
	//animation
	MixedAnimation manim;
	Animation *overrideAnim;
	Frame *animRoot;
	std::vector<uint> boneToFrame;
	float curTime;
	float curOvrTime;

	int currentColorStep;

	void attachTexDict(TexDictionary &t);
	void updateFrames(Frame *r);
	void updateGeometries(void);
	void applyAnim(Frame *f);
	void drawGeometry(int g);

public:
	void request(std::string model, std::string texdict);
	void release(void);
	int loadSynch(std::string model, std::string texdict);
	void unload(void);

	void attachClump(rw::Clump *c);
	void attachAnim(Animation *a);
	void attachMixedAnim(Animation *a, Animation *b, float f);
	void attachOverrideAnim(Animation *a);
	void draw(void);
	void drawAtomic(uint a);
	void drawFrame(Frame *f, bool recurse, bool transform);
	void setVertexColors(void);
	float getTime(void);
	void setTime(float t);
	float getOvrTime(void);
	void setOvrTime(float t);
	void resetFrames(void);
	bool hasModel(void);
	bool hasTextures(void);
	quat getPosition(void);

	void printFrames(int level, Frame *r);
	void dumpClump(bool detailed);
	std::vector<quat> getBoundingSpheres(void);

	Drawable(void);
	~Drawable(void);
};

#endif
