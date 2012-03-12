#ifndef DRAWABLE_H
#define DRAWABLE_H

#include "gta.h"
#include "texman.h"

#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <renderware.h>

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
};

/*
struct Texture {
	std::string name;
	std::string maskName;
	GLuint tex;
	int dictIndex;
};
*/

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
//	std::vector<Texture> texList;

	TexDictionary *texDict;

//	rw::TextureDictionary texDict;

	rw::Animation anim;
	Frame *animRoot;
	std::vector<uint> boneToFrame;
	int frame;
	int endFrame;

//	int searchModelTexture(std::string name);
//	int searchDictTexture(rw::TextureDictionary &dict, std::string name);
	void attachTexDict(TexDictionary &t);
	void updateFrames(Frame *r);
	void updateGeometries(void);
	void applyAnim(Frame *f);
	void drawGeometry(int g, bool drawTransp);
public:

	/* functions */
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
};

#endif
