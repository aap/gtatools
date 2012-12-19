#ifndef GTA_TEXMAN_H
#define GTA_TEXMAN_H

#include <renderware.h>
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
	TexDictionary *parent;
	bool isLoaded;
	bool isLoading;
	bool isGlobal;

	int load(bool synch);
	void attachTxd(rw::TextureDictionary *txd);
	void unload(void);
	Texture *get(std::string searchName);
	bool isHierarchyLoaded(void);
	~TexDictionary(void);
};

class TexManager
{
private:
	std::deque<TexDictionary *> txdList;
	std::deque<TexDictionary *> globalTxdList;
	uint add(std::string fileName, bool load, bool synch);
	uint find(std::string fileName);
public:
	TexDictionary *requestSynch(std::string fileName);
	TexDictionary *get(std::string fileName,
	                   bool load = true, bool synch = false);
	Texture *getGlobalTex(std::string texName);
	void release(std::string fileName);
	void addParentInfo(std::string child, std::string parent);
	void addGlobal(std::string fileName);
	void dump(void);
	TexManager(void);
	~TexManager(void);

	void dumpLoaded(void);
};

extern TexManager *texMan;

inline bool TexDictionary::isHierarchyLoaded(void)
{
	return parent ? (isLoaded && parent->isHierarchyLoaded())
	              : isLoaded;
}
#endif
