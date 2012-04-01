#include "gta.h"
#include "col.h"
#include <renderware.h>
using namespace std;
using namespace rw;

int CollisionModel::read(ifstream &in)
{
	char buffer[24];
	buffer[4] = '\0';
	in.read(buffer, 4);
	if (in.eof() || buffer[0] == '\0')
		return 0;
	string format = buffer;

	int version;

	if (format == "COLL") {
		version = 1;
	} else {
		cout << "unknown col format\n";
		return 0;
	}

	uint32 filesize = readUInt32(in);
	in.read(buffer, 24);
	name = buffer;
	stringToLower(name);
//	cout << name << endl;

	float32 dataf[10];
	in.read((char*)dataf, 10*sizeof(float32));
	if (version == 1) {
		boundingSphere.w = dataf[0];
		boundingSphere.x = dataf[1];
		boundingSphere.y = dataf[2];
		boundingSphere.z = dataf[3];
		min.x = dataf[4];
		min.y = dataf[5];
		min.z = dataf[6];
		max.x = dataf[7];
		max.y = dataf[8];
		max.z = dataf[9];
	} else {
		min.x = dataf[0];
		min.y = dataf[1];
		min.z = dataf[2];
		max.x = dataf[3];
		max.y = dataf[4];
		max.z = dataf[5];
		boundingSphere.x = dataf[6];
		boundingSphere.y = dataf[7];
		boundingSphere.z = dataf[8];
		boundingSphere.w = dataf[9];
	}

	if (version > 1) {
	}

	if (version == 1) {
		uint count = readUInt32(in);
		spheres.resize(count);
		for (uint i = 0; i < count; i++) {
			in.read((char*)dataf, 4*sizeof(float32));
			spheres[i].sphere.w = dataf[0];
			spheres[i].sphere.x = dataf[1];
			spheres[i].sphere.y = dataf[2];
			spheres[i].sphere.z = dataf[3];
			spheres[i].surf.material = readUInt8(in);
			spheres[i].surf.flag = readUInt8(in);
			spheres[i].surf.brightness = readUInt8(in);
			spheres[i].surf.light = readUInt8(in);
		}

		in.seekg(4, ios::cur);
		count = readUInt32(in);
		boxes.resize(count);
		for (uint i = 0; i < count; i++) {
			in.read((char*)dataf, 6*sizeof(float32));
			boxes[i].min.x = dataf[0];
			boxes[i].min.y = dataf[1];
			boxes[i].min.z = dataf[2];
			boxes[i].max.x = dataf[3];
			boxes[i].max.y = dataf[4];
			boxes[i].max.z = dataf[5];
			boxes[i].surf.material = readUInt8(in);
			boxes[i].surf.flag = readUInt8(in);
			boxes[i].surf.brightness = readUInt8(in);
			boxes[i].surf.light = readUInt8(in);
		}

		count = readUInt32(in);
		vertices.resize(count*3);
		in.read((char*)&vertices[0], count*3*sizeof(float32));

		count = readUInt32(in);
		faces.resize(count);
		uint32 datai[3];
		for (uint i = 0; i < count; i++) {
			in.read((char*)datai, 3*sizeof(uint32));
			faces[i].a = datai[0];
			faces[i].b = datai[1];
			faces[i].c = datai[2];
			faces[i].surf.material = readUInt8(in);
			faces[i].surf.flag = readUInt8(in);
			faces[i].surf.brightness = readUInt8(in);
			faces[i].surf.light = readUInt8(in);
		}
	} else {
	}
	return 1;
//	cout << hex << in.tellg() << endl;
}
