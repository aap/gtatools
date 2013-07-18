#ifndef _WIN32
  #include <sys/types.h>
  #include <dirent.h>
#endif

#include <cstdlib>
#include <signal.h>

#include <iostream>
#include <fstream>

#include "gta.h"
#include "gl.h"
#include "renderer.h"
#include "objects.h"
#include "world.h"
#include "enex.h"
#include "directory.h"
#include "drawable.h"
#include "camera.h"
#include "timecycle.h"
#include "water.h"
#include "texman.h"
#include "jobqueue.h"
#include "console.h"
#include "phys.h"
using namespace std;

/*
 * The Game
 */

string gamePath;
char *progname;
int game;
bool consoleVisible = false;
pthread_t oglThread;
volatile bool running;

void parseDat(ifstream &f);

Ped *player = 0;

void exitprog(void)
{
	running = false;
	normalJobs.wakeUp();
}

void cleanUp(void)
{
	SAFE_DELETE(cam);
	SAFE_DELETE(player);
	SAFE_DELETE(objectList);
	SAFE_DELETE(world);
	SAFE_DELETE(water);
	SAFE_DELETE(console);
	SAFE_DELETE(enexList);
	SAFE_DELETE(texMan);
	SAFE_DELETE(directory);
	SAFE_DELETE(renderer);

	drawable.unload();

	physShutdown();
}

int initGame(void)
{
	renderer = new Renderer;
	if (renderer->init()) {
		cerr << "render init failed\n";
		return 1;
	}

	directory = new Directory;
	texMan = new TexManager;
	world = new World;
	enexList = new EnexList;
	water = new Water;
	console = new Console;

	console->init();

	string datFileName = getPath("data/gta.dat");
	ifstream f(datFileName.c_str());
	if (!f.fail()) {
		game = GTASA;
		world->initSectors(quat(-3000, -3000, 0),
		                  quat(3000, 3000, 0), 12);
		objectList = new ObjectList(19000);
	} else {
		datFileName = getPath("data/gta_vc.dat");
		f.open(datFileName.c_str(), ios::binary);
		if (!f.fail()) {
			game = GTAVC;
			world->initSectors(quat(-2448, -2048, 0),
			                  quat(1648, 2048, 0), 10);
			objectList = new ObjectList(5000);
		} else {
			datFileName = getPath("data/gta3.dat");
			f.open(datFileName.c_str(), ios::binary);
			if (!f.fail()) {
				game = GTA3;
				world->initSectors(quat(-2048, -2048, 0),
						  quat(2048, 2048, 0), 10);
				objectList = new ObjectList(5000);
			} else {
				cerr << "couldn't open dat file\n";
				return 1;
			}
		}
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
	directory->addFromFile(dirFile, imgPath);
	dirFile.close();

	// load gta_int.img and player.img
	if (game == GTASA) {
		imgPath = getPath("models/gta_int.img");
		dirFile.open(imgPath.c_str(), ios::binary);
		if (dirFile.fail()) {
			cerr << "can't open img/dir file\n";
			return 1;
		}
		directory->addFromFile(dirFile, imgPath);
		dirFile.close();

		imgPath = getPath("models/player.img");
		dirFile.open(imgPath.c_str(), ios::binary);
		if (dirFile.fail()) {
			cerr << "can't open img/dir file\n";
			return 1;
		}
		directory->addFromFile(dirFile, imgPath);
		dirFile.close();
	}

	// try to open txd archive for optimized textures
	dirPath = getPath("models/txd.dir");
	imgPath = getPath("models/txd.img");
	// try to open .dir, if it fails open (v2) .img
	dirFile.open(dirPath.c_str(), ios::binary);
	if (!dirFile.fail()) {
		cout << "found txd archive, loading\n";
		directory->addFromFile(dirFile, imgPath);
		dirFile.close();
	} else {
		dirFile.open(imgPath.c_str(), ios::binary);
		if (!dirFile.fail()) {
			cout << "found txd archive, loading\n";
			directory->addFromFile(dirFile, imgPath);
			dirFile.close();
		}
	}

	// load other files
	directory->addFile(getPath("models/particle.txd"));

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


	player = new Ped;
	player->reset();
	if (game != GTASA) {
		player->modelName = "player";
		player->textureName = "player";
	} else {
		player->modelName = "cesar";
		player->textureName = "cesar";
	}
	player->loadSynch();

	cam = new Camera;

/*
	cam->setPitch(PI/8.0f-PI/2.0f);
	cam->setDistance(20.0f);
	cam->setAspectRatio(GLfloat(gl::width) / GLfloat(gl::height));
	cam->setTarget(quat(335.5654907, -159.0345306, 17.85120964));
*/

	cout << "associating lods\n";
	world->associateLods();

	cout << "populating islands\n";
	world->populateIslands();

	// load water
	cout << "loading water\n";
	if (game == GTA3 || game == GTAVC) {
		string fileName = getPath("data/waterpro.dat");
		ifstream f(fileName.c_str(), ios::binary);
		if (f.fail()) {
			cerr << "couldn't open waterpro.dat\n";
		} else {
			water->loadWaterpro(f);
			f.close();
		}
	} else {	// must be GTASA
		string fileName = getPath("data/water.dat");
		ifstream f(fileName.c_str());
		if (f.fail()) {
			cerr << "couldn't open water.dat\n";
		} else {
			water->loadWater(f);
			f.close();
		}
	}

	// load timecycle
	cout << "loading timecycle\n";
	string fileName = getPath("data/timecyc.dat");
	f.open(fileName.c_str());
	if (f.fail()) {
		cerr << "couldn't open timecyc.dat\n";
	} else {
		timeCycle.load(f);
		f.close();
	}

	fileName = getPath("anim/ped.ifp");
	f.open(fileName.c_str(), ios::binary);
	anpk.read(f);
	f.close();

	physInit();

	return 0;
}

void updateGame(float timeDiff)
{
	if (player)
		player->incTime(timeDiff);
	physStep(timeDiff);
}

void parseDat(ifstream &f)
{
	string type;
	string fileName;
	string line;
	vector<string> fields;
	int island;
	ifstream inFile;


	bool colsInitialized = false;
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
			objectList->readIde(inFile);
			inFile.close();

			size_t i = fileName.find_last_of(PSEP_C);
			objectList->findAndReadCol(fileName.substr(i+1));
		} else if (type == "IPL" || type == "MAPZONE") {
			if (!colsInitialized) {
				cout << "associating cols\n";
				objectList->associateCols();
				colsInitialized = true;
				cout << "continue with main dat\n";
			}

			inFile.open(fileName.c_str());
			if (inFile.fail()) {
				cerr << "couldn't open ipl " <<fileName<<endl;
				exit(1);
			}
			world->readIpl(inFile,
			     fileName.substr(fileName.find_last_of(PSEP_S)+1));
			inFile.close();
		} else if (type == "TEXDICTION") {
			directory->addFile(fileName);
			texMan->addGlobal(fileName);
		} else if (type == "MODELFILE") {
			directory->addFile(fileName);
		} else if (type == "IMG") {
			inFile.open(fileName.c_str(), ios::binary);
			if (inFile.fail()) {
				cerr << "couldn't open img " << fileName<<endl;
				exit(1);
			}
			directory->addFromFile(inFile, fileName);
			inFile.close();
		} else if (type == "COLFILE") {
			island = atoi(fields[1].c_str());
			inFile.open(fileName.c_str());
			if (inFile.fail()) {
				cerr << "couldn't open col " <<fileName<<endl;
				exit(1);
			}
			objectList->readCol(inFile, island);
			inFile.close();
		} else if (type == "SPLASH") {
		}
	}
}

bool isPointInBox(quat p, quat corner1, quat corner2)
{
	if (p.x >= corner1.x && p.x <= corner2.x &&
	    p.y >= corner1.y && p.y <= corner2.y &&
	    p.z >= corner1.z && p.z <= corner2.z) {
		return true;
	}
	return false;
}

bool isSphereInBox(quat s, quat corner1, quat corner2)
{
	if (s.x - s.w >= corner1.x && s.x + s.w <= corner2.x &&
	    s.y - s.w >= corner1.y && s.y + s.w <= corner2.y &&
	    s.z - s.w >= corner1.z && s.z + s.w <= corner2.z) {
		return true;
	}
	return false;
}

bool isSphereOutsideBox(quat s, quat corner1, quat corner2)
{
	if (s.x + s.w < corner1.x || s.x - s.w > corner2.x ||
	    s.y + s.w < corner1.y || s.y - s.w > corner2.y ||
	    s.z + s.w < corner1.z || s.z - s.w > corner2.z) {
		return true;
	}
	return false;
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
	for (size_t i = 0; i < subFolders.size(); i++) {

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

