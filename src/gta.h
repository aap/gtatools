#ifndef GTA_GTA_H
#define GTA_GTA_H

#ifdef _WIN32
  #include <windows.h>
  #undef ERROR // error in glm
  #define PSEP_C '\\'
  #define PSEP_S "\\"
#else
  #define PSEP_C '/'
  #define PSEP_S "/"
#endif

#include <pthread.h>
#include <string>
#include <vector>
#include "math.h"

#define THREADCHECK()\
	if (oglThread != pthread_self()) {\
		cout << "warning: using gl commands not in gl thread\t ";\
		cout << __PRETTY_FUNCTION__ << endl;\
	}

#define SAFE_DELETE(p)\
	delete p; p = 0;

enum {
	GTA3 = 1,
	GTAVC = 2,
	GTASA = 3
};

typedef unsigned char uchar;
typedef unsigned int uint;

extern char *progname;
extern std::string gamePath;
extern int game;
extern bool consoleVisible;
extern volatile bool running;
extern pthread_t oglThread;

class Ped;
extern Ped *player;

void exitprog(void);
void cleanUp(void);
int initGame(void);
void updateGame(float timeDiff);

bool isPointInBox(quat p, quat corner1, quat corner2);
bool isSphereInBox(quat p, quat corner1, quat corner2);
bool isSphereOutsideBox(quat s, quat corner1, quat corner2);
std::string getPath(std::string relative);
void stringToLower(std::string &);
void getFields(std::string &s, std::string sep, std::vector<std::string> &v);
void correctFileName(std::string &fileName);

#endif
