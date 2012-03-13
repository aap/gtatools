#include <iostream>
#include <fstream>
#include <cstring>

#include <renderware.h>
using namespace std;
using namespace rw;

typedef unsigned int uint;

/*
 * NOT FINISHED
 */

uint32 appendFile(uint32 length, ifstream &in, ofstream &out)
{
	uint32 written = 0;
	char buf[2048];

	memset(buf, 0, 2048);
	do {
		in.read(buf, 2048);
		out.write(buf, 2048);
		written += 2048;
	} while (written < length);
	return written;
}

int main(int argc, char *argv[])
{
	bool ver2f = false;
	if (argc < 2) {
		cout << "usage: " << argv[0] << " dir_file [img_file]\n";
		return 1;
	}

	if (argc <= 2)
		ver2f = true;
	cout << "Ver2? " << ver2f << endl;

	ifstream inDir;
	inDir.open(argv[1], ios::binary);
	if (inDir.fail()) {
		cerr << "Error: couldn't open " << argv[1] << endl;
		return 1;
	}

	ofstream outDir;
	outDir.open("gta.dir", ios::binary);
	if (inDir.fail()) {
		cerr << "Error: couldn't open gta.dir\n";
		return 1;
	}

	uint32 numEntries;
	if (ver2f) {
		inDir.seekg(4, ios::cur);
		numEntries = readUInt32(inDir);
	} else {
		inDir.seekg(0, ios::end);
		numEntries = inDir.tellg();
		inDir.seekg(0, ios::cur);
	}

	ifstream inImg;
	ofstream outImg;
	if (!ver2f) {
		inImg.open(argv[2], ios::binary);
		if (inImg.fail()) {
			cerr << "Error: couldn't open " << argv[2] << endl;
			return 1;
		}

		outImg.open("gta.img", ios::binary);
		if (inImg.fail()) {
			cerr << "Error: couldn't open gta.img\n";
			return 1;
		}
	} else {
//		inImg = inDir;
//		outImg = outDir;
	}

	uint32 imgPos = ver2f ? (((numEntries*32)+8)+2047)/2048 : 0;
	uint32 dirPos = 0;

	if (ver2f) {
		outDir.write("VER2", 4);
		writeUInt32(numEntries, outDir);
		dirPos += 8;
	}

	for (uint32 i = 0; i < numEntries; i++) {
		cout << i << " of " << numEntries << endl;
		inDir.seekg(dirPos, ios::beg);
		outDir.seekp(dirPos, ios::beg);
		uint32 start = readUInt32(inDir)*2048;
		uint32 length = readUInt32(inDir)*2048;
		char name[24];
		inDir.read(name, 24);

		uint32 newlen;
		if (ver2f) {
			outDir.seekp(imgPos*2048, ios::beg);
			inDir.seekg(start, ios::beg);
			newlen = appendFile(length, inDir, outDir);
		} else {
			cout << hex << start/2048 << " " << imgPos << " " << length<< endl;
			outImg.seekp(imgPos*2048, ios::beg);
			inImg.seekg(start, ios::beg);
			newlen = appendFile(length, inImg, outImg);
		}
//		newlen = length;
		newlen = (newlen+2047)/2048;

		outDir.seekp(dirPos, ios::beg);
		writeUInt32(imgPos, outDir);
		writeUInt32(newlen, outDir);
		outDir.write(name, 24);

		imgPos += newlen;
		dirPos += 32;
	}
}
