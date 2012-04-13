#ifndef GTA_TEXMAN_H
#define GTA_TEXMAN_H

#include <string>
#include <deque>
#include <GL/glew.h>
#include "gta.h"

struct Texture {
	std::string name;
	std::string maskName;
	bool hasAlpha;
	GLuint tex;
};

struct TexDictionary {
	std::vector<Texture> texList;
	std::string fileName;
	int refCount;
	bool loaded;
	TexDictionary *parent;
	bool isGlobal;

	int load(void);
	void unload(void);
	Texture *get(std::string searchName);
};

class TexManager
{
private:
	std::deque<TexDictionary *> txdList;
	std::deque<TexDictionary *> globalTxdList;
	uint add(std::string fileName, bool load);
	uint find(std::string fileName);
public:
	TexDictionary *get(std::string fileName, bool load = true);
	Texture *getGlobalTex(std::string texName);
	void release(std::string fileName);
	void addParentInfo(std::string child, std::string parent);
	void addGlobal(std::string fileName);
	void dump(void);
	TexManager(void);
};

extern TexManager texMan;

#endif
