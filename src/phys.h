#ifndef GTA_PHYS_H
#define GTA_PHYS_H
#include "gl.h"

void physInit(void);
void physShutdown(void);
void physStep(float time);

extern RefFrame *coltest;

#endif
