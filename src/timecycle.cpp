#include <cstdlib>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "gta.h"
#include "timecycle.h"

using namespace std;

TimeCycle timeCycle;

void Weather::mix(Weather &w1, Weather &w2, float a)
{
	amb = w1.amb*a + w2.amb*(1-a);
	ambObj = w1.ambObj*a + w2.ambObj*(1-a);
	ambBl = w1.ambBl*a + w2.ambBl*(1-a);
	ambObjBl = w1.ambObjBl*a + w2.ambObjBl*(1-a);
	dir = w1.dir*a + w2.dir*(1-a);
	skyTop = w1.skyTop*a + w2.skyTop*(1-a);
	skyBot = w1.skyBot*a + w2.skyBot*(1-a);
	sunCore = w1.sunCore*a + w2.sunCore*(1-a);
	sunCorona = w1.sunCorona*a + w2.sunCorona*(1-a);
	sunSz = w1.sunSz*a + w2.sunSz*(1-a);
	sprSz = w1.sprSz*a + w2.sprSz*(1-a);
	sprBght = w1.sprBght*a + w2.sprBght*(1-a);
	shdw = w1.shdw*a + w2.shdw*(1-a);
	lightShd = w1.lightShd*a + w2.lightShd*(1-a);
	treeShd = w1.treeShd*a + w2.treeShd*(1-a);
	poleShd = w1.poleShd*a + w2.poleShd*(1-a);
	farClp = w1.farClp*a + w2.farClp*(1-a);
	fogSt = w1.fogSt*a + w2.fogSt*(1-a);
	lightOnGround = w1.lightOnGround*a + w2.lightOnGround*(1-a);
	lowClouds = w1.lowClouds*a + w2.lowClouds*(1-a);
	topClouds = w1.topClouds*a + w2.topClouds*(1-a);
	bottomClouds = w1.bottomClouds*a + w2.bottomClouds*(1-a);
	blur = w1.blur*a + w2.blur*(1-a);
	water = w1.water*a + w2.water*(1-a);
	rgba1 = w1.rgba1*a + w2.rgba1*(1-a);
	rgba2 = w1.rgba2*a + w2.rgba2*(1-a);
	tint = w1.tint*a + w2.tint*(1-a);
	cloudAlpha = w1.cloudAlpha*a + w2.cloudAlpha*(1-a);
}

void Weather::readLine(vector<string> fields)
{
	int i = 0;
	amb = quat(1, atof(fields[i].c_str()),
		      atof(fields[i+1].c_str()),
		      atof(fields[i+2].c_str()));
	i += 3;
	if (game != GTA3) {
		ambObj = quat(1, atof(fields[i].c_str()),
		                 atof(fields[i+1].c_str()),
		                 atof(fields[i+2].c_str()));
		i += 3;
	} else {
		ambObj = amb;
	}
	if (game == GTAVC) {
		ambBl = quat(1, atof(fields[i].c_str()),
		                atof(fields[i+1].c_str()),
		                atof(fields[i+2].c_str()));
		i += 3;
		ambObjBl = quat(1, atof(fields[i].c_str()),
		                   atof(fields[i+1].c_str()),
		                   atof(fields[i+2].c_str()));
		i += 3;
	} else {
		ambBl = amb;
		ambObjBl = ambObj;
	}

	dir = quat(1, atof(fields[i].c_str()),
		      atof(fields[i+1].c_str()),
		      atof(fields[i+2].c_str()));
	i += 3;
	skyTop = quat(1, atof(fields[i].c_str()),
	                 atof(fields[i+1].c_str()),
	                 atof(fields[i+2].c_str()));
	i += 3;
	skyBot = quat(1, atof(fields[i].c_str()),
	                 atof(fields[i+1].c_str()),
	                 atof(fields[i+2].c_str()));
	i += 3;
	sunCore = quat(1, atof(fields[i].c_str()),
	                  atof(fields[i+1].c_str()),
	                  atof(fields[i+2].c_str()));
	i += 3;
	sunCorona = quat(1, atof(fields[i].c_str()),
	                    atof(fields[i+1].c_str()),
	                    atof(fields[i+2].c_str()));
	i += 3;

	sunSz = atof(fields[i++].c_str());
	sprSz = atof(fields[i++].c_str());
	sprBght = atof(fields[i++].c_str());
	shdw = atof(fields[i++].c_str());
	lightShd = atof(fields[i++].c_str());
	if (game == GTA3)
		treeShd = atof(fields[i++].c_str());
	else
		poleShd = atof(fields[i++].c_str());
	farClp = atof(fields[i++].c_str());
	fogSt = atof(fields[i++].c_str());
	lightOnGround = atof(fields[i++].c_str());

	lowClouds = quat(1, atof(fields[i].c_str()),
	                    atof(fields[i+1].c_str()),
	                    atof(fields[i+2].c_str()));
	i += 3;
	topClouds = quat(1, atof(fields[i].c_str()),
	                    atof(fields[i+1].c_str()),
	                    atof(fields[i+2].c_str()));
	i += 3;
	if (game != GTASA) {
		bottomClouds = quat(1, atof(fields[i].c_str()),
		                       atof(fields[i+1].c_str()),
		                       atof(fields[i+2].c_str()));
		i += 3;
	}
	if (game == GTAVC) {
		blur = quat(1, atof(fields[i].c_str()),
		               atof(fields[i+1].c_str()),
		               atof(fields[i+2].c_str()));
		i += 3;
	}
	if (game != GTA3) {
		water = quat(atof(fields[i+3].c_str()),
		             atof(fields[i].c_str()),
		             atof(fields[i+1].c_str()),
		             atof(fields[i+2].c_str()));
		i += 4;
	} else {
		water = quat(255, 255, 255, 255);
	}
	if (game == GTASA) {
		rgba1 = quat(atof(fields[i].c_str()),
		             atof(fields[i+1].c_str()),
		             atof(fields[i+2].c_str()),
		             atof(fields[i+3].c_str()));
		i += 4;
		rgba2 = quat(atof(fields[i].c_str()),
		             atof(fields[i+1].c_str()),
		             atof(fields[i+2].c_str()),
		             atof(fields[i+3].c_str()));
		i += 4;
	// TODO: these are 4 values actually
		cloudAlpha = atof(fields[i++].c_str());
	} else {
		rgba1 = quat(128, 128, 128, 128);
		rgba2 = quat(0, 0, 0, 0);
		cloudAlpha = 1.0f;
	}
	if (game == GTA3) {
		tint = quat(atof(fields[i+3].c_str()),
		            atof(fields[i].c_str()),
		            atof(fields[i+1].c_str()),
		            atof(fields[i+2].c_str()));
	}

	amb /= 255.0f;
	ambObj /= 255.0f;
	ambBl /= 255.0f;
	ambObjBl /= 255.0f;
	dir /= 255.0f;
	skyTop /= 255.0f;
	skyBot /= 255.0f;
	sunCore /= 255.0f;
	sunCorona /= 255.0f;
	lowClouds /= 255.0f;
	topClouds /= 255.0f;
	bottomClouds /= 255.0f;
	blur /= 255.0f;
	water /= 255.0f;
	rgba1 /= 255.0f;
	rgba1.w *= 2.0f;
	rgba2 /= 255.0f;
	rgba2.w *= 2.0f;
	tint /= 255.0f;
}

void TimeCycle::load(ifstream &f)
{
	string line;
	vector<string> fields;
//	Weather w;
	if (game == GTA3)
		numWeathers = 4;
	else if (game == GTAVC)
		numWeathers = 6;
	else	// GTASA
		numWeathers = 23;
	int sa[8] = { 0, 5, 6, 7, 12, 19, 20, 22 };

	for (int i = 0; i < numWeathers; i++) {
		if (game == GTASA) {
			int j = 0;
			while (j < 8) {
				getline(f, line);
				getFields(line, " \t", fields);
				if (fields.size() < 1 ||
				    fields[0].substr(0,2) == "//")
					continue;
				weatherList[i][sa[j]].readLine(fields);
				j++;
			}
			weatherList[i][1].mix(weatherList[i][0],
			                      weatherList[i][5], 1.0/5.0*4);
			weatherList[i][2].mix(weatherList[i][0],
			                      weatherList[i][5], 1.0/5.0*3);
			weatherList[i][3].mix(weatherList[i][0],
			                      weatherList[i][5], 1.0/5.0*2);
			weatherList[i][4].mix(weatherList[i][0],
			                      weatherList[i][5], 1.0/5.0*1);

			weatherList[i][8].mix(weatherList[i][7],
			                      weatherList[i][12], 1.0/5.0*4);
			weatherList[i][9].mix(weatherList[i][7],
			                      weatherList[i][12], 1.0/5.0*3);
			weatherList[i][10].mix(weatherList[i][7],
			                      weatherList[i][12], 1.0/5.0*2);
			weatherList[i][11].mix(weatherList[i][7],
			                      weatherList[i][12], 1.0/5.0*1);

			weatherList[i][13].mix(weatherList[i][12],
			                      weatherList[i][19], 1.0/7.0*6);
			weatherList[i][14].mix(weatherList[i][12],
			                      weatherList[i][19], 1.0/7.0*5);
			weatherList[i][15].mix(weatherList[i][12],
			                      weatherList[i][19], 1.0/7.0*4);
			weatherList[i][16].mix(weatherList[i][12],
			                      weatherList[i][19], 1.0/7.0*3);
			weatherList[i][17].mix(weatherList[i][12],
			                      weatherList[i][19], 1.0/7.0*2);
			weatherList[i][18].mix(weatherList[i][12],
			                      weatherList[i][19], 1.0/7.0*1);

			weatherList[i][21].mix(weatherList[i][20],
			                      weatherList[i][22], 0.5);

			weatherList[i][23].mix(weatherList[i][22],
			                      weatherList[i][0], 0.5);
		} else {
			int j = 0;
			while (j < 24) {
				getline(f, line);
				getFields(line, " \t", fields);
				if (fields.size() < 1 ||
				    fields[0].substr(0,2) == "//")
					continue;
				weatherList[i][j].readLine(fields);
				j++;
			}
		}
	}
}

void TimeCycle::calcCurrent(void)
{
	if (hour < 0 || hour >= 24)
		return;
	if (hour == 23)
		currentWeatherData.mix(weatherList[currentWeather][23],
		                       weatherList[currentWeather][0],
		                       1 - minute/60.0f);
	else
		currentWeatherData.mix(weatherList[currentWeather][hour],
		                       weatherList[currentWeather][hour+1],
		                       1 - minute/60.0f);
}

Weather *TimeCycle::getCurrentWeatherData(void)
{
	return &currentWeatherData;
}

int TimeCycle::getHour(void) { return hour; }
int TimeCycle::getMinute(void) { return minute; }
void TimeCycle::setHour(int h)
{
	if (h < 0)
		hour = 23;
	else if (h >= 24)
		hour = 0;
	else
		hour = h;
	updateColorStep();
}

void TimeCycle::setMinute(int m)
{
	if (m < 0) {
		minute = 59;
		setHour(hour-1);
	} else if (m >= 60) {
		minute = 0;
		setHour(hour+1);
	} else {
		minute = m;
	}
	updateColorStep();
}

int TimeCycle::getCurrentWeather(void)
{
	return currentWeather;
}

void TimeCycle::setCurrentWeather(int w)
{
	currentWeather = w % numWeathers;
	cout << currentWeather << endl;
}

void TimeCycle::setColorStep(int t)
{
	if (t >= 0 && t <= 5)
		colorStep = t;
}

int TimeCycle::getColorStep(void)
{
	return colorStep;
}

// kind of resembles the lighting in the game
void TimeCycle::updateColorStep(void)
{
	if (hour < 6)
		colorStep = 5;
	else if (hour == 6)
		colorStep = 5 - minute/10;
	else if (hour == 20)
		colorStep = minute/10;
	else if (hour > 20)
		colorStep = 5;
	else
		colorStep = 0;
}

TimeCycle::TimeCycle(void)
{
	hour = 12;
	colorStep = 0;
}
