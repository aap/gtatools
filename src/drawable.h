#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <renderware.h>

#include "gta.h"
#include "texman.h"
#include "math.h"

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

	rw::Animation anim;
	Frame *animRoot;
	std::vector<uint> boneToFrame;
	int frame;
	int endFrame;

	void attachTexDict(TexDictionary &t);
	void updateFrames(Frame *r);
	void updateGeometries(void);
	void applyAnim(Frame *f);
	void drawGeometry(int g, bool drawTransp);

public:
	int load(std::string model, std::string texdict);
	void unload(void);
	void attachClump(rw::Clump &c);	// better use a pointer
	void attachTexDict(rw::TextureDictionary &t);	// better use a pointer
	void attachAnim(rw::Animation &a);
	void nextFrame(void);
	void draw(bool drawTransp);
	void drawAtomic(int a, bool drawTransp);
	void drawFrame(int fi, bool drawTransp, bool recurse);
	void printFrames(int level, Frame *r);
	quat getBoundingSphere(void);
};

#endif