#ifndef GTA_H
#define GTA_H
#ifdef _WIN32
  #include <windows.h>
  #undef ERROR // error in glm
  #define PSEP_C '\\'
  #define PSEP_S "\\"
#else
  #define PSEP_C '/'
  #define PSEP_S "/"
#endif
#include <string>
#include <vector>
#include <GL/glew.h>

enum {
	GTA3 = 1,
	GTAVC = 2,
	GTASA = 3
};

typedef unsigned char uchar;
typedef unsigned int uint;

#include "drawable.h"
#include "quat.h"

extern char *progname;
extern std::string gamePath;
extern int game;
extern bool running;

extern Drawable drawable;

void initGame(void);
std::string getPath(std::string relative);
void stringToLower(std::string &);
void getFields(std::string &s, std::string sep, std::vector<std::string> &v);
void correctFileName(std::string &fileName);

#endif
