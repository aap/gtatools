#ifndef DIRECTORY_H
#define DIRECTORY_H
#include <string>
#include <vector>
#include <deque>
#include "gta.h"

struct DirectoryFile
{
	std::string fileName;
	rw::uint32 start;
	rw::uint32 length;
	std::string containingFile;
};

class Directory
{
private:
	std::deque<DirectoryFile *> fileList;
public:
	void addFromFile(std::ifstream &f, std::string container);
	void addFile(DirectoryFile *);
	void addFile(std::string path);
	void addFile(std::string name, rw::uint32 start, rw::uint32 length,
	             std::string container);
	int openFile(std::ifstream &f, uint i);
	int openFile(std::ifstream &f, std::string searchName);
	void dump(void);
};

extern Directory directory;

#endif
