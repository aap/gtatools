#include <pthread.h>
#include <renderware.h>
#include "gta.h"
#include "directory.h"
#include "texman.h"
#include "drawable.h"
#include "jobqueue.h"

using namespace std;

JobQueue normalJobs;
JobQueue glJobs;

void JobQueue::addJob(void (*func)(void*, void*), void *sender, void *data)
{
	Job j;
	j.func = func;
	j.sender = sender;
	j.data = data;
	pthread_mutex_lock(&mutex);
	pthread_mutex_lock(&condMutex);
	jobQueue.push_back(j);
	pthread_cond_broadcast(&cond);
	pthread_mutex_unlock(&condMutex);
	pthread_mutex_unlock(&mutex);
}

bool JobQueue::processJob(void)
{
	if (!jobQueue.empty()) {
		pthread_mutex_lock(&mutex);
		Job j = jobQueue[0];
		jobQueue.pop_front();
		pthread_mutex_unlock(&mutex);
		j.func(j.sender, j.data);
	}
	return !jobQueue.empty();
}

void JobQueue::waitForJobs(void)
{
	pthread_mutex_lock(&condMutex);
	if (jobQueue.empty())
		pthread_cond_wait(&cond, &condMutex);
	pthread_mutex_unlock(&condMutex);
}

void JobQueue::wakeUp(void)
{
	pthread_mutex_lock(&condMutex);
	pthread_cond_broadcast(&cond);
	pthread_mutex_unlock(&condMutex);
}

JobQueue::JobQueue(void)
{
	pthread_mutex_init(&(mutex), NULL);
	pthread_mutex_init(&(condMutex), NULL);
	pthread_cond_init(&(cond), NULL);
}


static void attachClump(void *sender, void *data);
static void attachTxd(void *sender, void *data);
static void deleteDrawable(void *sender, void *data);

void JobQueue::readDff(void *sender, void *data)
{
	string name = (char *) data;
	delete[] (char*)data;
	ifstream file;
	if (directory.openFile(file, name) == -1) {
		cout << "couldn't open " << name << endl;
		return;
	}

	rw::Clump *c = new rw::Clump;
	c->read(file);
	file.close();
	glJobs.addJob(attachClump, sender, c);
}

void JobQueue::readTxd(void *sender, void *data)
{
	string name = (char *) data;
	delete[] (char*)data;
	ifstream file;
	if (directory.openFile(file, name) == -1) {
		cout << "couldn't open " << name << endl;
		return;
	}

	rw::TextureDictionary *txd = new rw::TextureDictionary;
	txd->read(file);
	file.close();

	// convert to a sensible format
	for (uint i = 0; i < txd->texList.size(); i++) {
		if (txd->texList[i].platform == rw::PLATFORM_PS2)
			txd->texList[i].convertFromPS2();
		if (txd->texList[i].platform == rw::PLATFORM_XBOX)
			txd->texList[i].convertFromXbox();
		if (txd->texList[i].dxtCompression)
			txd->texList[i].decompressDxt();
		txd->texList[i].convertTo32Bit();
	}
	glJobs.addJob(attachTxd, sender, txd);
}

void JobQueue::deleteDrawable(void *sender, void *data)
{
	glJobs.addJob(::deleteDrawable, sender, 0);
}


static void deleteDrawable(void *sender, void *data)
{
	Drawable *d = (Drawable*)sender;
	delete d;
}

static void attachClump(void *sender, void *data)
{
	Drawable *d = (Drawable*)sender;
	rw::Clump *clp = (rw::Clump*)data;
	d->attachClump(clp);
}

static void attachTxd(void *sender, void *data)
{
	TexDictionary *txd = (TexDictionary*)sender;
	rw::TextureDictionary *rwtxd = (rw::TextureDictionary*)data;
	txd->attachTxd(rwtxd);
}

