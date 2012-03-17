#ifndef TEXMAN_H
#define TEXMAN_H
#include "gta.h"
#include <deque>

#include <renderware.h>

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

	int load(void);
	void unload(void);
	Texture *get(std::string searchName);
};

class TexManager
{
private:
	std::deque<TexDictionary *> txdList;
	uint add(std::string fileName, bool load);
	uint find(std::string fileName);
public:
	TexDictionary *get(std::string fileName, bool load = true);
	void release(std::string fileName);
	void addParentInfo(std::string child, std::string parent);
	void dump(void);
	TexManager(void);
};

extern TexManager texMan;

#endif
