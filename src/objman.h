#ifndef GTA_OBJMAN_H
#define GTA_OBJMAN_H

#include <deque>
#include "objects.h"

class ObjManager
{
private:
	std::deque<WorldObject*> requestQueue;
public:
	void request(WorldObject *mp);
	void loadSingleObject(void);
};

extern ObjManager objMan;

#endif
