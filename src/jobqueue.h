#ifndef GTA_FILEREADER_H
#define GTA_FILEREADER_H

#include <pthread.h>
#include <deque>
#include <iostream>

struct Job {
	void (*func)(void*, void*);
	void *sender;
	void *data;
};

class JobQueue
{
private:
	std::deque<Job> jobQueue;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	pthread_mutex_t condMutex;
public:
	void addJob(void (*func)(void*, void*), void *sender, void *data);
	bool processJob(void);
	void waitForJobs(void);
	void wakeUp(void);
	JobQueue(void);

	static void readDff(void *sender, void *data);
	static void readTxd(void *sender, void *data);
	static void deleteDrawable(void *sender, void *data);
};

extern JobQueue normalJobs;
extern JobQueue glJobs;

#endif
