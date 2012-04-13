#include <cstring>
#include <fstream>
#include <vector>
#include <string>

#include <renderware.h>

#include "gta.h"
#include "directory.h"

using namespace std;

Directory directory;

void Directory::addFromFile(ifstream &f, string container)
{
	char fourcc[5];
	rw::uint32 numEntries;

	memset(fourcc, 0, 5);
	f.read(fourcc, 4*sizeof(char));
	if (strcmp(fourcc, "VER2") == 0) {
		numEntries = rw::readUInt32(f);
	} else {
		f.seekg(0, ios::end);
		numEntries = f.tellg();
		numEntries /= 32;
		f.seekg(0, ios::beg);
	}
	for (uint i = 0; i < numEntries; i++) {
		rw::uint32 start = rw::readUInt32(f)*2048;
		rw::uint32 length = rw::readUInt32(f)*2048;
		char fileName[25];
		memset(fileName, 0, 25);
		f.read(fileName, 24*sizeof(char));

		std::string name = fileName;

		addFile(name, start, length, container);
	}
}

void Directory::addFile(DirectoryFile *dirFile)
{
	if (fileList.size() > 0) {
		string s = dirFile->fileName;

		int min, max, mid;
		min = 0; max = fileList.size() - 1;

		while (min <= max) {
			mid = (min+max) / 2;
			if (fileList[mid]->fileName == s) {
				fileList[mid]->fileName = dirFile->fileName;
				fileList[mid]->start = dirFile->start;
				fileList[mid]->length = dirFile->length;
				fileList[mid]->containingFile =
					dirFile->containingFile;
				return;
			}
			if (fileList[mid]->fileName > s)
				max = mid - 1;
			else if (fileList[mid]->fileName < s)
				min = mid + 1;
		}
		fileList.insert(fileList.begin()+min, dirFile);
		return;
	}

	fileList.push_back(dirFile);
}

void Directory::addFile(string path)
{
	vector<string> subFolders;

	getFields(path, PSEP_S, subFolders);
	string name = subFolders[subFolders.size()-1];
	ifstream f(path.c_str());
	if (f.fail()) {
		cerr << "couldn't open file " << path << endl;
		return;
	}
	f.seekg(0, ios::end);
	uint size = f.tellg();
	f.close();

	addFile(name, 0, size, path);
}

void Directory::addFile(string name, rw::uint32 start, rw::uint32 length,
                        string container)
{
	DirectoryFile *dirFile = new DirectoryFile;
	dirFile->fileName = name;
	stringToLower(dirFile->fileName);
	dirFile->start = start;
	dirFile->length = length;
	dirFile->containingFile = container;
	addFile(dirFile);
}

int Directory::openFile(ifstream &f, string searchName)
{
	string s;

	s = searchName;
	stringToLower(s);

	uint min, max, mid;
	min = 0; max = fileList.size() - 1;

	while (min <= max) {
		mid = (min+max) / 2;
		if (fileList[mid]->fileName == s) {
			openFile(f, mid);
			return 0;
		}
		if (fileList[mid]->fileName > s)
			max = mid - 1;
		else if (fileList[mid]->fileName < s)
			min = mid + 1;
	}

	return -1;
}

int Directory::openFile(ifstream &f, uint i)
{
	f.open(fileList[i]->containingFile.c_str(), ios::binary);
	if (f.fail())
		return -1;
	f.seekg(fileList[i]->start, ios::beg);
	return 0;
}

void Directory::dump(void)
{
	for (uint i = 0; i < fileList.size(); i++) {
		cout << fileList[i]->fileName << " ";
		cout << hex << fileList[i]->start << " ";
		cout << hex << fileList[i]->length << " ";
		cout << fileList[i]->containingFile << endl;
	}
}
