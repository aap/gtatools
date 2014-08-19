#include <cstdlib>

#include <iostream>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <renderware.h>

#include "gta.h"
#include "math.h"
#include "animation.h"
#include "drawable.h"

using namespace rw;
using namespace std;

// I'm not too pround of this, but it works
//#ifdef DEBUG
#if 1234
	#define READ_SECTION(x)\
		fourcc = readUInt32(ifp);\
		if (fourcc != x) {\
			cerr << "error: no " << hex << x << " found " << hex << ifp.tellg() << endl;\
			exit(1);\
		}\
		size = readUInt32(ifp);
	#define SECTION_DECLARATION\
		uint32 fourcc, size;
	#define SECTION_DECLARATION_2\
		uint32 fourcc, size;
#else
	#define READ_SECTION(x)\
		readUInt32(ifp);\
		size = readUInt32(ifp);
	#define SECTION_DECLARATION\
		uint32 size;
	#define SECTION_DECLARATION_2\
		uint32 fourcc, size;
#endif

enum {
	ANP3 = 0x33504e41,
	ANPK = 0x4b504e41,
	INFO = 0x4f464e49,
	NAME = 0x454d414e,
	DGAN = 0x4e414744,
	CPAN = 0x4e415043,
	ANMS = 0x4d494e41
};

void AnimPackage::read(ifstream &ifp)
{
	SECTION_DECLARATION_2
	fourcc = readUInt32(ifp);
	size = readUInt32(ifp);

	if (fourcc == ANPK) {
		READ_SECTION(INFO);

		uint32 numAnims = readUInt32(ifp);
		animList.resize(numAnims);
		size -= sizeof(uint32);
		size = (size+0x3) & ~0x3;
		char *buf = new char[size];
		ifp.read(buf, size);
		name = buf;
		delete[] buf;

		for (uint32 i = 0; i < numAnims; i++)
			animList[i].read_1(ifp);
	} else if (fourcc == ANP3) {
		char buf[24];
		ifp.read(buf, 24);
		name = buf;
		uint32 numAnims = readUInt32(ifp);
		animList.resize(numAnims);

		for (uint32 i = 0; i < numAnims; i++)
			animList[i].read_3(ifp);
	} else {
		cout << "no known ifp file\n";
	}
}

void Animation::read_1(ifstream &ifp)
{
	SECTION_DECLARATION

	READ_SECTION(NAME);

	size = (size+0x3) & ~0x3;
	char *buf = new char[size];
	ifp.read(buf, size);
	name = buf;
	stringToLower(name);
	delete[] buf;

	READ_SECTION(DGAN);
	uint32 end = size + ifp.tellg();

	READ_SECTION(INFO);

	uint32 numObjs = readUInt32(ifp);
	size -= sizeof(uint32);
	size = (size+0x3) & ~0x3;
	ifp.seekg(size, ios::cur);

	endTime = 0.0f;
	for (uint32 i = 0; i < numObjs && ifp.tellg() < end; i++) {
		objList.resize(objList.size()+1);
		objList[i].read_1(ifp);
//		int last = objList[i].frames-1;
		size_t last = objList[i].frmList.size();
		if(last != 0 && objList[i].frmList[last-1].timeKey > endTime)
			endTime = objList[i].frmList[last-1].timeKey;
	}
	for (size_t i = 0; i < objList.size(); i++) {
		AnimObj &ao = objList[i];
		for (int j = 0; j < ao.frames; j++)
			ao.frmList[j].timeKey /= endTime;
	}
}

void Animation::read_3(ifstream &ifp)
{
	char buf[24];
	ifp.read(buf, 24);
	name = buf;
	stringToLower(name);
	uint32 numObjs = readUInt32(ifp);
	ifp.seekg(4, ios::cur);	// frameSize
	ifp.seekg(4, ios::cur);

	endTime = 0.0f;
	objList.resize(numObjs);
	for (uint32 i = 0; i < numObjs; i++) {
		objList[i].read_3(ifp);
		int last = objList[i].frames-1;
		if (objList[i].frmList[last].timeKey > endTime)
			endTime = objList[i].frmList[last].timeKey;
	}
	for (size_t i = 0; i < objList.size(); i++) {
		AnimObj &ao = objList[i];
		for (int j = 0; j < ao.frames; j++)
			ao.frmList[j].timeKey /= endTime;
	}
}

void AnimObj::read_1(std::ifstream &ifp)
{
	SECTION_DECLARATION_2

	READ_SECTION(CPAN);

	READ_SECTION(ANMS);
	char *buf = new char[28];
	ifp.read(buf, 28);
	name = buf;
	stringToLower(name);
	delete[] buf;
	frames = readInt32(ifp); // this value isn't always right it seems
	unknown = readInt32(ifp);
	next = readInt32(ifp);
	prev = readInt32(ifp);
	ifp.seekg(size-28-4*sizeof(int32), ios::cur);

	if(frames == 0)
		return;

	// KFRM
	fourcc = readUInt32(ifp);
	size = readUInt32(ifp);
	uint32 end = size + ifp.tellg();
	for (uint32 i = 0; ifp.tellg() < end; i++) {
		frmList.resize(frmList.size()+1);
		frmList[i].read_1(fourcc, ifp);
	}
}

void AnimObj::read_3(std::ifstream &ifp)
{
	char buf[24];
	ifp.read(buf, 24);
	name = buf;
	stringToLower(name);
	uint32 frmType = readUInt32(ifp);
	frames = readUInt32(ifp);
	ifp.seekg(4, ios::cur); // boneId

	frmList.resize(frames);
	for (int32 i = 0; i < frames; i++)
		frmList[i].read_3(ifp, frmType);
}

void KeyFrame::read_1(uint32 fourcc, ifstream &ifp)
{
	float32 data[4];

	if (fourcc == KR00) {
		ifp.read(reinterpret_cast<char*>(&data), 4*sizeof(float32));
		rot.w = data[3];
		rot.x = -data[0];
		rot.y = -data[1];
		rot.z = -data[2];
	} else if (fourcc == KRT0) {
		ifp.read(reinterpret_cast<char*>(&data), 4*sizeof(float32));
		rot.w = data[3];
		rot.x = -data[0];
		rot.y = -data[1];
		rot.z = -data[2];
		ifp.read(reinterpret_cast<char*>(&data), 3*sizeof(float32));
		pos.x = data[0];
		pos.y = data[1];
		pos.z = data[2];
	} else if (fourcc == KRTS) {
		ifp.read(reinterpret_cast<char*>(&data), 4*sizeof(float32));
		rot.w = data[3];
		rot.x = -data[0];
		rot.y = -data[1];
		rot.z = -data[2];
		ifp.read(reinterpret_cast<char*>(&data), 3*sizeof(float32));
		pos.x = data[0];
		pos.y = data[1];
		pos.z = data[2];
		ifp.read(reinterpret_cast<char*>(&data), 3*sizeof(float32));
		scale.x = data[0];
		scale.y = data[1];
		scale.z = data[2];
	}
	timeKey = readFloat32(ifp);
	type = fourcc;
}

void KeyFrame::read_3(ifstream &ifp, uint32 frmType)
{
	int16 data[5];
	ifp.read(reinterpret_cast<char*>(&data), 5*sizeof(int16));
	rot.x = data[0]/4096.0f;
	rot.y = data[1]/4096.0f;
	rot.z = data[2]/4096.0f;
	rot.w = data[3]/4096.0f;
	timeKey = data[4]/60.0f;
	type = KR00;

	if (frmType == 4) {
		ifp.read(reinterpret_cast<char*>(&data), 3*sizeof(int16));
		pos.x = data[0]/1024.0f;
		pos.y = data[1]/1024.0f;
		pos.z = data[2]/1024.0f;
		type = KRT0;
	}
}
