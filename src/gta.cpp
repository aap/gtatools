#ifndef _WIN32
  #include <sys/types.h>
  #include <dirent.h>
#endif

#include <cstdlib>

#include <renderware.h>

#include "gta.h"
#include "directory.h"
#include "world.h"
#include "gl.h"
#include "texman.h"

using namespace std;
using namespace rw;

/*
 * The Game
 */

string gamePath;
char *progname;
int game;
bool running;

Drawable drawable;

void parseDat(ifstream &f)
{
	string type;
	string fileName;
	string line;
	vector<string> fields;
	int island;
	ifstream inFile;


	while (!f.eof()) {
		getline(f, line);
		getFields(line, " \t", fields);

		if (fields.size() < 1 || fields[0][0] == '#')
			continue;

		type = fields[0];

		if (type == "COLFILE")
			fileName = fields[2];
		else
			fileName = fields[1];
		fileName = getPath(fileName);
		if (type == "IDE") {
			inFile.open(fileName.c_str());
			if (inFile.fail()) {
				cerr << "couldn't open ide " <<fileName<<endl;
				exit(1);
			}
			objectList.readIde(inFile);
			inFile.close();
		} else if (type == "IPL" || type == "MAPZONE") {
			inFile.open(fileName.c_str());
			if (inFile.fail()) {
				cerr << "couldn't open ipl " <<fileName<<endl;
				exit(1);
			}
			world.readIpl(inFile,
			     fileName.substr(fileName.find_last_of(PSEP_S)+1));
			inFile.close();
		} else if (type == "TEXDICTION") {
			directory.addFile(fileName);
			texMan.addGlobal(fileName);
		} else if (type == "MODELFILE") {
			directory.addFile(fileName);
		} else if (type == "IMG") {
			inFile.open(fileName.c_str(), ios::binary);
			if (inFile.fail()) {
				cerr << "couldn't open img " << fileName<<endl;
				exit(1);
			}
			directory.addFromFile(inFile, fileName);
			inFile.close();
		} else if (type == "COLFILE") {
			island = atoi(fields[1].c_str());
			inFile.open(fileName.c_str());
			if (inFile.fail()) {
				cerr << "couldn't open col " <<fileName<<endl;
				exit(1);
			}
			objectList.readCol(inFile, island);
			inFile.close();
		} else if (type == "SPLASH") {
		}
	}
}

int main(int argc, char *argv[])
{
	progname = argv[0];
	if (argc < 4) {
//	if (argc < 2) {
		cerr << "too few arguments\n";
		return 1;
	}
	gamePath = argv[1];
	string datFileName = getPath("data/gta.dat");
	ifstream f(datFileName.c_str());
	if (f.fail()) {
		datFileName = getPath("data/gta_vc.dat");
		f.open(datFileName.c_str(), ios::binary);
		if (f.fail()) {
			datFileName = getPath("data/gta3.dat");
			f.open(datFileName.c_str(), ios::binary);
			if (f.fail()) {
				cerr << "couldn't open dat file\n";
				return 0;
			}
			game = GTA3;
			objectList.init(5000);
		} else {
			game = GTAVC;
			objectList.init(5000);
		}
	} else {
		game = GTASA;
		objectList.init(19000);
	}

	f.close();

	cout << "game path: " << gamePath << endl;

	// open gta3 archive
	cout << "loading gta3 archive\n";
	ifstream dirFile;

	string dirPath = getPath("models/gta3.dir");
	string imgPath = getPath("models/gta3.img");
	// try to open .dir, if it fails open (v2) .img
	dirFile.open(dirPath.c_str(), ios::binary);
	if (dirFile.fail()) {
		dirFile.open(imgPath.c_str(), ios::binary);
		if (dirFile.fail()) {
			cerr << "can't open img/dir file\n";
			return 1;
		}
	}
	directory.addFromFile(dirFile, imgPath);
	dirFile.close();

	// load gta_int.img and player.img
	if (game == GTASA) {
		imgPath = getPath("models/gta_int.img");
		dirFile.open(imgPath.c_str(), ios::binary);
		if (dirFile.fail()) {
			cerr << "can't open img/dir file\n";
			return 1;
		}
		directory.addFromFile(dirFile, imgPath);
		dirFile.close();

		imgPath = getPath("models/player.img");
		dirFile.open(imgPath.c_str(), ios::binary);
		if (dirFile.fail()) {
			cerr << "can't open img/dir file\n";
			return 1;
		}
		directory.addFromFile(dirFile, imgPath);
		dirFile.close();
	}

	// try to open txd archive for optimized textures
	dirPath = getPath("models/txd.dir");
	imgPath = getPath("models/txd.img");
	// try to open .dir, if it fails open (v2) .img
	dirFile.open(dirPath.c_str(), ios::binary);
	if (!dirFile.fail()) {
		cout << "found txd file\n";
		directory.addFromFile(dirFile, imgPath);
		dirFile.close();
	} else {
		dirFile.open(imgPath.c_str(), ios::binary);
		if (!dirFile.fail()) {
			cout << "found txd file\n";
			directory.addFromFile(dirFile, imgPath);
			dirFile.close();
		}
	}

	// TODO: load other files
	directory.addFile(getPath("models/particle.txd"));

	// load default dat file
	string datFileName2 = getPath("data/default.dat");
	cout << "loading default dat\n";
	ifstream datFile(datFileName2.c_str());
	if (!datFile.fail()) {
		parseDat(datFile);
		datFile.close();
	}

	// load main dat file
	cout << "loading main dat\n";
	datFile.open(datFileName.c_str());
	if (datFile.fail()) {
		cerr << "couldn't open " << datFileName << endl;
		return 1;
	}
	parseDat(datFile);
	datFile.close();

	gl::start(&argc, argv);

	return 0;
}

/*
 * Game globals and general utility functions
 */

string getPath(string relative)
{
	correctFileName(relative);
	return gamePath + PSEP_S + relative;
}

void stringToLower(string &s)
{
	for (uint i = 0; i < s.length(); i++)
		if (isupper(s[i]))
			s[i] = tolower(s[i]);
}

// split s into fields separated by any character in sep
// ugly
void getFields(string &s, string sep, vector<string> &v)
{
	v.clear();
	// remove carriage return
	if (s[s.size()-1] == 0x0d)
		s = s.substr(0, s.size()-1);

	// remove trailing whitespace
	size_t j = s.find_last_not_of(" \t");
	if (j != string::npos) {
		s.resize(j+1);
	} else {
		if (s[s.size()-1] == ' ' || s[s.size()-1] == '\t')
			return;
			
	}

	if (s.size() == 0)
		return;

	j = s.find_first_of(sep);
	if (j == string::npos) {
		v.push_back(s);
		return;
	}

	size_t i = 0;
	while (j != string::npos) {
		v.push_back(s.substr(i, j-i));
		i = ++j;
		if ((j = s.find_first_not_of(" \t", i)) != string::npos)
			i = j;
		j = s.find_first_of(sep, i);
		if (j == string::npos) {
			j = s.find_first_of(" \t", i);
			if (j == string::npos)
				v.push_back(s.substr(i));
			else
				v.push_back(s.substr(i, j-i));
		}
	}

	// remove trailing whitespace
	for (i = 0; i < v.size(); i++) {
		j = v[i].find_first_of(" \t");
		v[i] = v[i].substr(0, j);
	}
}

// compare real file names to stored ones and construct correctly cased path
void correctPathCase(string &fileName)
{
#ifndef _WIN32
	DIR *dir;
	struct dirent *dirent;

	vector<string> subFolders;
	getFields(fileName, PSEP_S, subFolders);

	string pathSoFar = gamePath;
	for (uint i = 0; i < subFolders.size(); i++) {

		if ((dir = opendir(pathSoFar.c_str())) == 0) {
			cerr << "couldn't open dir: " << pathSoFar << endl;
			return;
		}
		while ((dirent = readdir(dir)) != 0) {
			string fsName = dirent->d_name;
			stringToLower(fsName);
			string tmp = subFolders[i];
			stringToLower(tmp);
			if (tmp == fsName) {
				pathSoFar += PSEP_S;
				pathSoFar += dirent->d_name;
				break;
			}
		}
		closedir(dir);
		if (dirent == 0) {
			return;
		}

	}

	// remove gamePath + /
	fileName = pathSoFar.substr(gamePath.size() + 1);
#endif
}

void correctFileName(string &fileName)
{
	for (uint i = 0; i < fileName.length(); i++) {
		if (fileName[i] == '/' || fileName[i] == '\\')
			fileName[i] = PSEP_C;
	}
#ifndef _WIN32
	correctPathCase(fileName);
#endif
}

