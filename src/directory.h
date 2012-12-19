#ifndef GTA_DIRECTORY_H
#define GTA_DIRECTORY_H
#include <string>
#include <deque>
#include <fstream>
#include <renderware.h>

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

	~Directory(void);
};

extern Directory *directory;

#endif
