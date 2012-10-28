#ifndef GTA_IFP_H
#define GTA_IFP_H
#include <string>
#include <vector>
#include <fstream>
#include <renderware.h>
#include "math.h"

enum {
	KR00 = 0x3030524b,
	KRT0 = 0x3054524b,
	KRTS = 0x5354524b
};

struct KeyFrame
{
	rw::uint32 type;
	quat rot;
	quat pos;
	quat scale;
	rw::float32 timeKey;

	/* functions */
	void read_1(rw::uint32 fourcc, std::ifstream &ifp);
	void read_3(std::ifstream &ifp, rw::uint32 type);
};

struct AnimObj
{
	std::string name;
	rw::int32 frames;
	rw::int32 unknown;
	rw::int32 next;
	rw::int32 prev;

	std::vector<KeyFrame> frmList;

	/* functions */
	void read_1(std::ifstream &ifp);
	void read_3(std::ifstream &ifp);
	void interpolate(float t, KeyFrame &key);
};

struct Frame;

struct Animation
{
	std::string name;
	std::vector<AnimObj> objList;
	float endTime;

	/* functions */
	void getKeyframe(float t, std::string name, KeyFrame &kf);
	void apply(float curTime, Frame *f, bool recurse = true);
	void read_1(std::ifstream &ifp);
	void read_3(std::ifstream &ifp);
	void clear(void);
};

struct AnimPackage
{
	std::vector<Animation> animList;
	std::string name;

	/* functions */
	void read(std::ifstream &ifp);
	void clear(void);
};

class MixedAnimation
{
private:
	Animation *anim1;
	Animation *anim2;
	float mixFactor;
public:
	float endTime;
	void attachAnims(Animation *a, Animation *b, float f);
	void apply(float t, Frame *f);

	MixedAnimation(void);
	MixedAnimation(Animation *a, Animation *b, float f);
};

#endif
