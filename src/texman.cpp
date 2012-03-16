#include <iostream>

#include "gta.h"
#include "texman.h"
#include "directory.h"

using namespace std;

TexManager TexMan;

/*
 * TexManager
 */

TexDictionary *TexManager::get(string fileName)
{
	stringToLower(fileName);

	uint pos = find(fileName);
	if (pos != 0) {
		txdList[pos]->refCount++;
		return txdList[pos];
	}

	uint i = add(fileName);
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
			delete txdList[pos];
			txdList.erase(txdList.begin()+pos);
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

uint TexManager::add(string fileName)
{
	ifstream txdf;
	if (directory.openFile(txdf, fileName) == -1) {
		cout << "couldn't open " << fileName << endl;
		return 0;
	}
	rw::TextureDictionary rwtxd;
	rwtxd.read(txdf);
	txdf.close();

	// convert to a sensible format
	for (uint i = 0; i < rwtxd.texList.size(); i++) {
		if (rwtxd.texList[i].platform == rw::PLATFORM_PS2)
			rwtxd.texList[i].convertFromPS2();
		if (rwtxd.texList[i].platform == rw::PLATFORM_XBOX)
			rwtxd.texList[i].convertFromXbox();
		if (rwtxd.texList[i].dxtCompression)
			rwtxd.texList[i].decompressDxt();
		rwtxd.texList[i].convertTo32Bit();
	}

	TexDictionary *txd = new TexDictionary;
	txd->load(rwtxd);
	rwtxd.clear();	// TODO: Why doesn't the deconstructor do this?
	txd->fileName = fileName;
	txd->refCount = 1;

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
//	cout << child << " " << parent << endl;
}

/*
 * TexDictionary
 */

Texture *TexDictionary::get(string searchName)
{
	for (uint i = 0; i < texList.size(); i++)
		if (texList[i].name == searchName)
			return &texList[i];
	return 0;
}

void TexDictionary::load(rw::TextureDictionary &txd)
{
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
}

void TexDictionary::unload(void)
{
	for (uint i = 0; i < texList.size(); i++)
		glDeleteTextures(1, &texList[i].tex);
}
