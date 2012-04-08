#ifndef RENDERER_H
#define RENDERER_H

#include "gta.h"
#include "world.h"

struct RenderListObject {
	Instance *inst;
	int atomic;
};

class Renderer
{
	std::vector<RenderListObject> opaqueRenderList;
	std::vector<RenderListObject> transp1RenderList;
	std::vector<RenderListObject> transp2RenderList;
public:
	void init(void);
	void renderScene(void);
	void addOpaqueObject(Instance *ip, int a);
	void addTransp1Object(Instance *ip, int a);
	void addTransp2Object(Instance *ip, int a);

	void renderOpaque(void);
	void renderTransp1(void);
	void renderTransp2(void);

	bool doZones;
	bool doTextures;
	bool doFog;
	bool doVertexColors;
	bool doCol;
	bool doTrails;
	bool doBFC;
	float lodMult;
};

extern Renderer renderer;

#endif
