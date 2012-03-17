#include <iostream>

#include "gta.h"
#include "texman.h"
#include "directory.h"

using namespace std;

TexManager texMan;

/*
 * TexManager
 */

TexDictionary *TexManager::get(string fileName, bool load)
{
	stringToLower(fileName);

	uint pos = find(fileName);
	if (pos != 0) {
		if (load) {
			if (!txdList[pos]->loaded) {
				txdList[pos]->load();
				if (txdList[pos]->parent && 
				    !txdList[pos]->parent->loaded) {
					txdList[pos]->parent->load();
					txdList[pos]->parent->refCount++;
				}
			}
			txdList[pos]->refCount++;
		}
		return txdList[pos];
	}

	uint i = add(fileName, load);
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
//			delete txdList[pos];
//			txdList.erase(txdList.begin()+pos);
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

uint TexManager::add(string fileName, bool load)
{
	TexDictionary *txd = new TexDictionary;
	txd->loaded = false;
	txd->parent = 0;
	txd->fileName = fileName;
	if (load)
		if (txd->load() != 0)
			return 0;

	if (txdList.size() > 0) {
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
	return txdList.size()-1;
}

TexManager::TexManager(void)
{
	TexDictionary *t = new TexDictionary;
	t->fileName = "NOTFOUND";
	txdList.push_back(t);
}

void TexManager::addParentInfo(std::string child, std::string parent)
{
	TexDictionary *txd = get(child, false);
	txd->parent = get(parent, false);
}

void TexManager::dump(void)
{
	for (uint i = 0; i < txdList.size(); i++) {
		cout << txdList[i]->fileName<<" "<<txdList[i]->loaded << endl;
		if (txdList[i]->parent)
			cout << " " << txdList[i]->parent->fileName << " " <<
			        txdList[i]->parent->loaded << endl;
	}
}

/*
 * TexDictionary
 */

Texture *TexDictionary::get(string searchName)
{
	for (uint i = 0; i < texList.size(); i++)
		if (texList[i].name == searchName)
			return &texList[i];
	if (parent)
		return parent->get(searchName);
	return 0;
}

int TexDictionary::load(void)
{
	ifstream txdf;
	if (directory.openFile(txdf, fileName) == -1) {
		cout << "couldn't open " << fileName << endl;
		return 1;
	}
	rw::TextureDictionary txd;
	txd.read(txdf);
	txdf.close();

	// convert to a sensible format
	for (uint i = 0; i < txd.texList.size(); i++) {
		if (txd.texList[i].platform == rw::PLATFORM_PS2)
			txd.texList[i].convertFromPS2();
		if (txd.texList[i].platform == rw::PLATFORM_XBOX)
			txd.texList[i].convertFromXbox();
		if (txd.texList[i].dxtCompression)
			txd.texList[i].decompressDxt();
		txd.texList[i].convertTo32Bit();
	}

	glActiveTexture(GL_TEXTURE0);
	for (uint i = 0; i < txd.texList.size(); i++) {
		rw::NativeTexture &nt = txd.texList[i];
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
		texList.push_back(t);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	txd.clear();	// TODO: Why doesn't the deconstructor do this?
	loaded = true;
	return 0;
}

void TexDictionary::unload(void)
{
	for (uint i = 0; i < texList.size(); i++)
		glDeleteTextures(1, &texList[i].tex);
	loaded = false;
}
