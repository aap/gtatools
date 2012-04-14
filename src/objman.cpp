#include <pthread.h>
#include "objects.h"
#include "objman.h"

using namespace std;

ObjManager objMan;

pthread_mutex_t requestMutex = PTHREAD_MUTEX_INITIALIZER;

void ObjManager::request(WorldObject *op)
{
/*
	// this is incredibly stupid
	pthread_mutex_lock(&requestMutex);
	if (requestQueue.size() > 0) {
		int min, max, mid;
		min = 1; max = requestQueue.size() - 1;

		while (min <= max) {
			mid = (min+max) / 2;
			if (requestQueue[mid]->id == op->id) {
				pthread_mutex_unlock(&requestMutex);
				return;
			}
			if (requestQueue[mid]->id > op->id)
				max = mid - 1;
			else if (requestQueue[mid]->id < op->id)
				min = mid + 1;
		}
		requestQueue.insert(requestQueue.begin()+min, op);
	} else {
		requestQueue.push_back(op);
	}
	pthread_mutex_unlock(&requestMutex);
*/

	if (op->isRequested)
		return;
	op->isRequested = true;
	pthread_mutex_lock(&requestMutex);
	requestQueue.push_front(op);
	pthread_mutex_unlock(&requestMutex);
}

void ObjManager::loadSingleObject(void)
{
	if (requestQueue.size() > 0) {
		pthread_mutex_lock(&requestMutex);
		WorldObject *op = requestQueue[0];
		requestQueue.pop_front();
		pthread_mutex_unlock(&requestMutex);
		op->load();
		op->isRequested = false;
		op->isFreshlyLoaded = true;
	}
}
