#ifndef GTA_RENDERER_H
#define GTA_RENDERER_H

#include <vector>
#include <deque>
#include "pipeline.h"

class Instance;
class Drawable;
extern Drawable drawable;

struct RenderListObject {
	Instance *inst;
	int atomic;
};

class Renderer
{
private:
	std::deque<RenderListObject> opaqueRenderList;
	std::vector<RenderListObject> transp1RenderList;
	std::vector<RenderListObject> transp2RenderList;

	gl::Pipeline *simplePipe, *gtaPipe;
	GLuint rectvbo;

public:
	int init(void);
	int reloadShaders(void);

	void renderScene(void);
	void addOpaqueObject(Instance *ip, int a);
	void addTransp1Object(Instance *ip, int a);
	void addTransp2Object(Instance *ip, int a);

	void renderOpaque(void);
	void renderTransp1(void);
	void renderTransp2(void);
	Renderer(void);
	~Renderer(void);

	bool doZones;
	bool doTextures;
	bool doFog;
	bool doVertexColors;
	bool doCol;
	bool doTrails;
	bool doBFC;
	float lodMult;
	float globalAlpha;

	GLuint whiteTex;

	bool drawWire;
	bool drawTransparent;
	bool wasTransparent;

	bool freeze;

	bool doReloadShaders;
};

extern Renderer *renderer;

#endif
