#ifndef GTA_TIMECYCLE_H
#define GTA_TIMECYCLE_H

#include <string>
#include <vector>
#include <fstream>
#include "math.h"

struct Weather {
public:
	quat amb;
	quat ambObj;		// vc and sa
	quat ambBl;		// vc
	quat ambObjBl;		// vc
	quat dir;
	quat skyTop;
	quat skyBot;
	quat sunCore;
	quat sunCorona;
	float sunSz;
	float sprSz;
	float sprBght;
	float shdw;
	float lightShd;
	float treeShd;		// 3
	float poleShd;		// vc and sa
	float farClp;
	float fogSt;
	float lightOnGround;
	quat lowClouds;
	quat topClouds;
	quat bottomClouds;	// 3 and vc
	quat blur;		// vc
	quat water;		// vc and sa
	quat rgba1;		// sa
	quat rgba2;		// sa
	quat tint;		// 3
	float cloudAlpha;	// sa

	void mix(Weather &w1, Weather &w2, float a);
	void readLine(std::vector<std::string> fields);
};

class TimeCycle
{
private:
	Weather weatherList[23][24];
	Weather currentWeatherData;
	int currentWeather;
	int hour, minute;
	int colorStep;	// this handles switching between day and night colors

	void readWeatherLine(Weather &w, std::vector<std::string> fields);

public:
	void load(std::ifstream &f);
	void calcCurrent(void);
	Weather *getCurrentWeatherData(void);
	int getHour(void);
	int getMinute(void);
	void setHour(int);
	void setMinute(int);
	void setColorStep(int);
	int getColorStep(void);
	void updateColorStep(void);
	TimeCycle(void);
};

extern TimeCycle timeCycle;

#endif

