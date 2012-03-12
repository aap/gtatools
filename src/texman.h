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

	void load(rw::TextureDictionary &txd);
	void unload(void);
	Texture *get(std::string searchName);
};

class TexManager
{
private:
	std::deque<TexDictionary *> txdList;
	uint add(std::string fileName);
	uint find(std::string fileName);
public:
	TexDictionary *get(std::string fileName);
	void release(std::string fileName);
	TexManager(void);
};

extern TexManager TexMan;

#endif
