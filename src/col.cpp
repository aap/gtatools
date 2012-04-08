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
	} else if (format == "COL2") {
		version = 2;
	} else if (format == "COL3") {
		version = 3;
	} else {
		cout << "unknown col format\n";
		cout << hex << in.tellg() << endl;
		return 0;
	}

	uint filestart = in.tellg();
	uint32 filesize = readUInt32(in);
	uint32 fileend = filestart+filesize+4;
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

	uint numSpheres = 0;
	uint numBoxes = 0;
	uint numVertices = 0;
	uint numFaces = 0;
	uint flags;
	uint sphereOffset;
	uint boxOffset;
	uint vertexOffset;
	uint faceOffset;

	uint numShadFaces = 0;
	uint numShadVertices = 0;
	uint shadVertexOffset;
	uint shadFaceOffset;

	if (version > 1) {
		numSpheres = readUInt16(in);
		numBoxes = readUInt16(in);
		numFaces = readUInt32(in);
		flags = readUInt32(in);
		sphereOffset = readUInt32(in);
		boxOffset = readUInt32(in);
		in.seekg(4, ios::cur);
		vertexOffset = readUInt32(in);
		faceOffset = readUInt32(in);
		in.seekg(4, ios::cur);

		numVertices = (faceOffset-vertexOffset)/(3*sizeof(uint16));

		if (version == 3) {
			numShadFaces = readUInt32(in);
			shadVertexOffset = readUInt32(in);
			shadFaceOffset = readUInt32(in);
			numShadVertices = (shadFaceOffset-shadVertexOffset) /
			                  (3*sizeof(uint16));
		}
	}

	if (version == 1) {
		numSpheres = readUInt32(in);
		spheres.resize(numSpheres);
		for (uint i = 0; i < numSpheres; i++) {
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
		numBoxes = readUInt32(in);
		boxes.resize(numBoxes);
		for (uint i = 0; i < numBoxes; i++) {
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

		numVertices = readUInt32(in);
		vertices.resize(numVertices*3);
		in.read((char*)&vertices[0], numVertices*3*sizeof(float32));

		numFaces = readUInt32(in);
		faces.resize(numFaces);
		uint32 datai[3];
		for (uint i = 0; i < numFaces; i++) {
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
		in.seekg(filestart+sphereOffset, ios::beg);
		spheres.resize(numSpheres);
		for (uint i = 0; i < numSpheres; i++) {
			in.read((char*)dataf, 4*sizeof(float32));
			spheres[i].sphere.w = dataf[3];
			spheres[i].sphere.x = dataf[0];
			spheres[i].sphere.y = dataf[1];
			spheres[i].sphere.z = dataf[2];
			spheres[i].surf.material = readUInt8(in);
			spheres[i].surf.flag = readUInt8(in);
			spheres[i].surf.brightness = readUInt8(in);
			spheres[i].surf.light = readUInt8(in);
		}

		in.seekg(filestart+boxOffset, ios::beg);
		boxes.resize(numBoxes);
		for (uint i = 0; i < numBoxes; i++) {
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

		int16 datai[3];
		// read faces first to make calculating the vertex count easier
		in.seekg(filestart+faceOffset, ios::beg);
		if (flags & 8) {
			uint32 pos = in.tellg();
			in.seekg(pos-sizeof(uint32), ios::beg);
			uint32 numFaceGroups = readUInt32(in);

			pos = in.tellg();
			in.seekg(pos-sizeof(uint32)-numFaceGroups*28,ios::beg);
			pos = in.tellg();
			numVertices = (pos-(vertexOffset+filestart)) /
			              (3*sizeof(uint16));
			faceGroups.resize(numFaceGroups);
			for (uint i = 0; i < numFaceGroups; i++) {
				in.read((char*)dataf,6*sizeof(float32));
				faceGroups[i].min.x = dataf[0];
				faceGroups[i].min.y = dataf[1];
				faceGroups[i].min.z = dataf[2];
				faceGroups[i].max.x = dataf[3];
				faceGroups[i].max.y = dataf[4];
				faceGroups[i].max.z = dataf[5];
				faceGroups[i].start = readUInt16(in);
				faceGroups[i].end = readUInt16(in);
			}
			in.seekg(sizeof(uint32), ios::cur);
		}
		faces.resize(numFaces);
		for (uint i = 0; i < numFaces; i++) {
			in.read((char*)datai, 3*sizeof(uint16));
			faces[i].a = datai[0];
			faces[i].b = datai[1];
			faces[i].c = datai[2];
			faces[i].surf.material = readUInt8(in);
			faces[i].surf.flag = 0;
			faces[i].surf.brightness = 0;
			faces[i].surf.light = readUInt8(in);
		}

		in.seekg(filestart+vertexOffset, ios::beg);
		vertices.resize(numVertices*3);
		for (uint i = 0; i < numVertices; i++) {
			in.read((char*)datai, 3*sizeof(int16));
			vertices[i*3+0] = datai[0] / 128.0f;
			vertices[i*3+1] = datai[1] / 128.0f;
			vertices[i*3+2] = datai[2] / 128.0f;
		}


		in.seekg(filestart+shadVertexOffset, ios::beg);
		shadVertices.resize(numShadVertices*3);
		for (uint i = 0; i < numShadVertices; i++) {
			in.read((char*)datai, 3*sizeof(int16));
			shadVertices[i*3+0] = datai[0] / 128.0f;
			shadVertices[i*3+1] = datai[1] / 128.0f;
			shadVertices[i*3+2] = datai[2] / 128.0f;
		}

		in.seekg(filestart+shadFaceOffset, ios::beg);
		shadFaces.resize(numShadFaces);
		for (uint i = 0; i < numShadFaces; i++) {
			in.read((char*)datai, 3*sizeof(uint16));
			shadFaces[i].a = datai[0];
			shadFaces[i].b = datai[1];
			shadFaces[i].c = datai[2];
			shadFaces[i].surf.material = readUInt8(in);
			shadFaces[i].surf.flag = 0;
			shadFaces[i].surf.brightness = 0;
			shadFaces[i].surf.light = readUInt8(in);
		}
	}
	in.seekg(fileend, ios::beg);
//	cout << hex << in.tellg() << endl << endl;
	return 1;
}
