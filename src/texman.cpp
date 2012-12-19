#include <GL/glew.h>

#include <renderware.h>

#include <iostream>
#include <string>
#include <cstring>

#include "gta.h"
#include "directory.h"
#include "texman.h"
#include "jobqueue.h"

using namespace std;

#include <ctime>

TexManager *texMan;

/*
 * TexManager
 */

void TexManager::dumpLoaded(void)
{
	for (size_t i = 0; i < txdList.size(); i++) {
		if (txdList[i]->isLoaded)
			cout << txdList[i]->fileName << endl;
	}
}

TexDictionary *TexManager::requestSynch(std::string fileName)
{
	return get(fileName, true, true);
}

TexDictionary *TexManager::get(string fileName, bool load, bool synch)
{
	stringToLower(fileName);

	uint pos = find(fileName);
	if (pos != 0) {
		if (load) {
			if (!txdList[pos]->isLoaded) {
				txdList[pos]->load(synch);
				if (txdList[pos]->parent && 
				    !txdList[pos]->parent->isLoaded) {
					txdList[pos]->parent->load(synch);
					txdList[pos]->parent->refCount++;
				}
			}
			txdList[pos]->refCount++;
		}
		return txdList[pos];
	}

	uint i = add(fileName, load, synch);
	if (load)
		txdList[i]->refCount = 1;
	return txdList[i];
}

void TexManager::release(string fileName)
{
	stringToLower(fileName);

	uint pos = find(fileName);
	if (pos != 0) {
		txdList[pos]->refCount--;
		if (txdList[pos]->refCount == 0) {
			txdList[pos]->unload();
			if (txdList[pos]->parent)
				release(txdList[pos]->parent->fileName);
		}
	}
}

uint TexManager::find(string fileName)
{
	string s;

	s = fileName;
	stringToLower(s);

	uint min, max, mid;
	min = 1;	// don't start at 0 (NOTFOUND)
	max = txdList.size() - 1;

	while (min <= max) {
		mid = (min+max) / 2;
		if (txdList[mid]->fileName == s)
			return mid;
		if (txdList[mid]->fileName > s)
			max = mid - 1;
		else if (txdList[mid]->fileName < s)
			min = mid + 1;
	}
	return 0;
}

uint TexManager::add(string fileName, bool load, bool synch)
{
	TexDictionary *txd = new TexDictionary;
	txd->isLoaded = false;
	txd->isLoading = false;
	txd->isGlobal = false;
	txd->parent = 0;
	txd->fileName = fileName;
	if (load)
		txd->load(synch);

	if (!txdList.empty()) {
		int min, max, mid;
		min = 1; max = txdList.size() - 1;

		while (min <= max) {
			mid = (min+max) / 2;
			if (txdList[mid]->fileName == txd->fileName) {
				cout << "this shouldn't happen\n";
				txd->unload();
				delete txd;
				return mid;
			}
			if (txdList[mid]->fileName > txd->fileName)
				max = mid - 1;
			else if (txdList[mid]->fileName < txd->fileName)
				min = mid + 1;
		}
		txdList.insert(txdList.begin()+min, txd);
		return min;
	}

	txdList.push_back(txd);
//	return txdList.size()-1;
	return 0;
}

void TexManager::addParentInfo(string child, string parent)
{
	TexDictionary *txd = get(child, false);
	txd->parent = get(parent, false);
}

void TexManager::addGlobal(std::string fileName)
{
	TexDictionary *txd = new TexDictionary;
	txd->isLoaded = false;
	txd->isLoading = false;
	txd->isGlobal = true;
	txd->parent = 0;
	stringToLower(fileName);
	size_t pos = fileName.find_last_of("/");
	if (pos != string::npos)
		fileName = fileName.substr(pos+1);
	txd->fileName = fileName;
	globalTxdList.push_back(txd);
}

Texture *TexManager::getGlobalTex(std::string texName)
{
	for (size_t i = 0; i < globalTxdList.size(); i++) {
		TexDictionary *txd = globalTxdList[i];
		if (!txd->isLoaded)
			txd->load(true);
		Texture *tex = txd->get(texName);
		if (tex != 0)
			return tex;
	}
	return 0;
}

void TexManager::dump(void)
{
	for (size_t i = 0; i < txdList.size(); i++) {
		cout << txdList[i]->fileName<<" "<<txdList[i]->isLoaded << endl;
		if (txdList[i]->parent)
			cout << " " << txdList[i]->parent->fileName << " " <<
				txdList[i]->parent->isLoaded << endl;
	}
	cout << "globals:\n";
	for (size_t i = 0; i < globalTxdList.size(); i++) {
		cout << globalTxdList[i]->fileName<<" "<<globalTxdList[i]->isLoaded << endl;
	}
}

TexManager::TexManager(void)
{
	TexDictionary *t = new TexDictionary;
	t->fileName = "NOTFOUND";
	txdList.push_back(t);
}

TexManager::~TexManager(void)
{
	for (size_t i = 0; i < txdList.size(); i++)
		delete txdList[i];
	for (size_t i = 0; i < globalTxdList.size(); i++)
		delete globalTxdList[i];
	txdList.clear();
	globalTxdList.clear();
}

/*
 * TexDictionary
 */

Texture *TexDictionary::get(string texName)
{
	// try to find texture
	// if not found, try parent
	// if still not found, try global texture
	for (size_t i = 0; i < texList.size(); i++)
		if (texList[i].name == texName)
			return &texList[i];
	Texture *t = 0;
	if (parent)
		t = parent->get(texName);
	if (t == 0 && !isGlobal)
		t = texMan->getGlobalTex(texName);
	return t;
}

int TexDictionary::load(bool synch)
{
	if (isLoading)
		return 0;
	isLoading = true;
	if (!synch) {
		char *str = new char[fileName.size()+1];
		strcpy(str, fileName.c_str());
		normalJobs.addJob(JobQueue::readTxd, this, str);
		return 0;
	}
	ifstream txdf;
	if (directory->openFile(txdf, fileName) == -1) {
		cout << "couldn't open " << fileName << endl;
		return -1;
	}
	rw::TextureDictionary *txd = new rw::TextureDictionary;
	txd->read(txdf);
	txdf.close();

	// convert to a sensible format
	for (size_t i = 0; i < txd->texList.size(); i++) {
		if (txd->texList[i].platform == rw::PLATFORM_PS2)
			txd->texList[i].convertFromPS2();
		if (txd->texList[i].platform == rw::PLATFORM_XBOX)
			txd->texList[i].convertFromXbox();
		if (txd->texList[i].dxtCompression)
			txd->texList[i].decompressDxt();
		txd->texList[i].convertTo32Bit();
	}

	attachTxd(txd);
	return 0;
}

void TexDictionary::attachTxd(rw::TextureDictionary *txd)
{
	THREADCHECK();
	glActiveTexture(GL_TEXTURE0);
	for (size_t i = 0; i < txd->texList.size(); i++) {
		rw::NativeTexture &nt = txd->texList[i];
		Texture t;
		t.name = nt.name;
		t.maskName = nt.maskName;
		stringToLower(t.name);
		stringToLower(t.maskName);
		t.hasAlpha = nt.hasAlpha;
		glGenTextures(1, &t.tex);
		glBindTexture(GL_TEXTURE_2D, t.tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

//		clock_t oldtime = clock();
		if ((nt.rasterFormat & rw::RASTER_MASK) == rw::RASTER_8888 ||
		    !(nt.rasterFormat & (rw::RASTER_PAL8 | rw::RASTER_PAL4))) {
			glTexImage2D(GL_TEXTURE_2D, 0, 4,
			             nt.width[0], nt.height[0], 0, GL_BGRA,
			             GL_UNSIGNED_BYTE, nt.texels[0]);
		} else if ((nt.rasterFormat&rw::RASTER_MASK)==rw::RASTER_888 ||
		    !(nt.rasterFormat & (rw::RASTER_PAL8 | rw::RASTER_PAL4))) {
			glTexImage2D(GL_TEXTURE_2D, 0, 4,
			             nt.width[0], nt.height[0], 0, GL_RGBA,
			             GL_UNSIGNED_BYTE, nt.texels[0]);
		} else {
			cout << "texture \"" << nt.name << "\" not bound" <<
			", rasterformat: " << hex << nt.rasterFormat << endl;
			glDeleteTextures(1, &t.tex);
			t.tex = 0;
		}
//		clock_t newtime = clock();
//		double diff = (newtime-oldtime) * 1000/CLOCKS_PER_SEC;

		texList.push_back(t);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
//	txd->clear();	// TODO: Why doesn't the deconstructor do this?
	delete txd;
	isLoaded = true;
	isLoading = false;
}

void TexDictionary::unload(void)
{
	THREADCHECK();
	for (size_t i = 0; i < texList.size(); i++)
		glDeleteTextures(1, &texList[i].tex);
	texList.clear();
	isLoaded = false;
}

/*
bool TexDictionary::isHierarchyLoaded(void)
{
	return parent ? (isLoaded && parent->isHierarchyLoaded()) : isLoaded;
}
*/

TexDictionary::~TexDictionary(void)
{
	THREADCHECK();
	for (size_t i = 0; i < texList.size(); i++)
		glDeleteTextures(1, &texList[i].tex);
	texList.clear();
}
