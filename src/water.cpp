#include <cstdlib>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <GL/glew.h>

#include "gta.h"
#include "gl.h"
#include "pipeline.h"
#include "renderer.h"
#include "timecycle.h"
#include "texman.h"
#include "water.h"

using namespace std;

Water water;

void Water::draw(void)
{
	THREADCHECK();
	Weather *w = timeCycle.getCurrentWeatherData();
	glm::vec4 matCol(w->water.x, w->water.y, w->water.z, w->water.w);
	gl::state.matColor = matCol;
	gl::state.updateMaterial();

	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);

	glVertexAttrib4f(gl::in_Color, 1.0f, 1.0f, 1.0f, 1.0f);
	glVertexAttrib3f(gl::in_Normal, 0.0f, 0.0f, 0.0f);
	if (renderer->doTextures)
		glBindTexture(GL_TEXTURE_2D, tex);
	else
		glBindTexture(GL_TEXTURE_2D, gl::whiteTex);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(gl::in_Vertex);
	glEnableVertexAttribArray(gl::in_TexCoord);
	glVertexAttribPointer(gl::in_Vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(gl::in_TexCoord, 2, GL_FLOAT, GL_FALSE, 0,
	                      (GLvoid*)(vertices.size()*3/5*sizeof(GLfloat)));
	glDrawArrays(GL_TRIANGLES, 0, vertices.size()*3/5/3);
	glDisableVertexAttribArray(gl::in_Vertex);
	glDisableVertexAttribArray(gl::in_TexCoord);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Water::loadWater(ifstream &f)
{
	THREADCHECK();
	string line;
	getline(f, line);
	if (line != "processed") {
		cout << "unknown format of SA water.dat\n";
		return;
	}

	TexDictionary *txd = texMan.requestSynch("particle.txd");
	Texture *t = txd->get("waterclear256");
	tex = t->tex;

	vector<GLfloat> texCoords;
	GLfloat x[4], y[4], z[4], f1[4], f2[4], f3[4], f4[4];
	GLuint param;
	vector<string> fields;
	while (!f.eof()) {
		getline(f, line);
		getFields(line, " \t", fields);

		for (uint i = 0; i < 3; i++ ) {
			x[i] = atof(fields[i*7+0].c_str());
			y[i] = atof(fields[i*7+1].c_str());
			z[i] = atof(fields[i*7+2].c_str());
			f1[i] = atof(fields[i*7+3].c_str());
			f2[i] = atof(fields[i*7+4].c_str());
			f3[i] = atof(fields[i*7+5].c_str());
			f4[i] = atof(fields[i*7+6].c_str());
		}
		if (fields.size() > 22) {
			x[3] = atof(fields[21].c_str());
			y[3] = atof(fields[22].c_str());
			z[3] = atof(fields[23].c_str());
			f1[3] = atof(fields[24].c_str());
			f2[3] = atof(fields[25].c_str());
			f3[3] = atof(fields[26].c_str());
			f4[3] = atof(fields[27].c_str());
		}

		param = atoi(fields[28].c_str());
		if (param == 0 || param == 2)
			continue;
		vertices.push_back(x[0]);
		vertices.push_back(y[0]);
		vertices.push_back(z[0]);
		vertices.push_back(x[1]);
		vertices.push_back(y[1]);
		vertices.push_back(z[1]);
		vertices.push_back(x[2]);
		vertices.push_back(y[2]);
		vertices.push_back(z[2]);
		texCoords.push_back(x[0]/32.0f);
		texCoords.push_back(y[0]/32.0f);
		texCoords.push_back(x[1]/32.0f);
		texCoords.push_back(y[1]/32.0f);
		texCoords.push_back(x[2]/32.0f);
		texCoords.push_back(y[2]/32.0f);
		if (fields.size() > 22) {
			vertices.push_back(x[1]);
			vertices.push_back(y[1]);
			vertices.push_back(z[1]);
			vertices.push_back(x[2]);
			vertices.push_back(y[2]);
			vertices.push_back(z[2]);
			vertices.push_back(x[3]);
			vertices.push_back(y[3]);
			vertices.push_back(z[3]);
			texCoords.push_back(x[1]/32.0f);
			texCoords.push_back(y[1]/32.0f);
			texCoords.push_back(x[2]/32.0f);
			texCoords.push_back(y[2]/32.0f);
			texCoords.push_back(x[3]/32.0f);
			texCoords.push_back(y[3]/32.0f);
		}
	}
	vertices.insert(vertices.end(), texCoords.begin(), texCoords.end());

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(GLfloat),
	             &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Water::loadWaterpro(std::ifstream &f)
{
	THREADCHECK();
	float palette[48];
	unsigned char indiceshi[128*128];
	unsigned char indiceslo[64*64];

	f.seekg(4, ios::cur);
	f.read(reinterpret_cast<char*>(&palette), 48*sizeof(float));
	f.seekg(0x300, ios::cur);
	f.read(reinterpret_cast<char*>(&indiceslo), 64*64);
	f.read(reinterpret_cast<char*>(&indiceshi), 128*128);

	float patchSize = 32;
	float base[2];
	TexDictionary *txd = texMan.requestSynch("particle.txd");
	if (game == GTA3) {
		base[0] = -2048;
		base[1] = -2048;
		Texture *t = txd->get("water_old");
		tex = t->tex;
	} else {	// must be GTAVC
		base[0] = -2448;
		base[1] = -2048;
		Texture *t = txd->get("waterclear256");
		tex = t->tex;
	}

	vector<GLfloat> texCoords;
	for (uint x = 0; x < 128; x++) {
		for (uint y = 0; y < 128; y++) {
			if (indiceshi[x*128+y] != 0x80) {
				float x0 = x*patchSize+base[0];
				float y0 = y*patchSize+base[1];
				float z = palette[indiceshi[x*128+y]];
				vertices.push_back(x0);
				vertices.push_back(y0);
				vertices.push_back(z);

				vertices.push_back(x0+patchSize);
				vertices.push_back(y0);
				vertices.push_back(z);

				vertices.push_back(x0);
				vertices.push_back(y0+patchSize);
				vertices.push_back(z);

				vertices.push_back(x0);
				vertices.push_back(y0+patchSize);
				vertices.push_back(z);

				vertices.push_back(x0+patchSize);
				vertices.push_back(y0);
				vertices.push_back(z);

				vertices.push_back(x0+patchSize);
				vertices.push_back(y0+patchSize);
				vertices.push_back(z);

				texCoords.push_back(0.0f);
				texCoords.push_back(0.0f);

				texCoords.push_back(1.0f);
				texCoords.push_back(0.0f);

				texCoords.push_back(0.0f);
				texCoords.push_back(1.0f);

				texCoords.push_back(0.0f);
				texCoords.push_back(1.0f);

				texCoords.push_back(1.0f);
				texCoords.push_back(0.0f);

				texCoords.push_back(1.0f);
				texCoords.push_back(1.0f);
			}
		}
	}

	float z = palette[indiceshi[0]];

	// western patch
	vertices.push_back(base[0]-2048);
	vertices.push_back(base[1]-2048);
	vertices.push_back(z);
	texCoords.push_back((base[0]-2048)/patchSize);
	texCoords.push_back((base[1]-2048)/patchSize);

	vertices.push_back(base[0]);
	vertices.push_back(base[1]-2048);
	vertices.push_back(z);
	texCoords.push_back((base[0])/patchSize);
	texCoords.push_back((base[1]-2048)/patchSize);

	vertices.push_back(base[0]-2048);
	vertices.push_back(base[1]+4096+2048);
	vertices.push_back(z);
	texCoords.push_back((base[0]-2048)/patchSize);
	texCoords.push_back((base[1]+4096+2048)/patchSize);

	vertices.push_back(base[0]-2048);
	vertices.push_back(base[1]+4096+2048);
	vertices.push_back(z);
	texCoords.push_back((base[0]-2048)/patchSize);
	texCoords.push_back((base[1]+4096+2048)/patchSize);

	vertices.push_back(base[0]);
	vertices.push_back(base[1]-2048);
	vertices.push_back(z);
	texCoords.push_back((base[0])/patchSize);
	texCoords.push_back((base[1]-2048)/patchSize);

	vertices.push_back(base[0]);
	vertices.push_back(base[1]+4096+2048);
	vertices.push_back(z);
	texCoords.push_back((base[0])/patchSize);
	texCoords.push_back((base[1]+4096+2048)/patchSize);

	// eastern patch
	vertices.push_back(base[0]+4096);
	vertices.push_back(base[1]-2048);
	vertices.push_back(z);
	texCoords.push_back((base[0]+4096)/patchSize);
	texCoords.push_back((base[1]-2048)/patchSize);

	vertices.push_back(base[0]+4096+2048);
	vertices.push_back(base[1]-2048);
	vertices.push_back(z);
	texCoords.push_back((base[0]+4096+2048)/patchSize);
	texCoords.push_back((base[1]-2048)/patchSize);

	vertices.push_back(base[0]+4096);
	vertices.push_back(base[1]+4096+2048);
	vertices.push_back(z);
	texCoords.push_back((base[0]+4096)/patchSize);
	texCoords.push_back((base[1]+4096+2048)/patchSize);

	vertices.push_back(base[0]+4096);
	vertices.push_back(base[1]+4096+2048);
	vertices.push_back(z);
	texCoords.push_back((base[0]+4096)/patchSize);
	texCoords.push_back((base[1]+4096+2048)/patchSize);

	vertices.push_back(base[0]+4096+2048);
	vertices.push_back(base[1]-2048);
	vertices.push_back(z);
	texCoords.push_back((base[0]+4096+2048)/patchSize);
	texCoords.push_back((base[1]-2048)/patchSize);

	vertices.push_back(base[0]+4096+2048);
	vertices.push_back(base[1]+4096+2048);
	vertices.push_back(z);
	texCoords.push_back((base[0]+4096+2048)/patchSize);
	texCoords.push_back((base[1]+4096+2048)/patchSize);

	// southern patch
	vertices.push_back(base[0]);
	vertices.push_back(base[1]-2048);
	vertices.push_back(z);
	texCoords.push_back((base[0])/patchSize);
	texCoords.push_back((base[1]-2048)/patchSize);

	vertices.push_back(base[0]+4096);
	vertices.push_back(base[1]-2048);
	vertices.push_back(z);
	texCoords.push_back((base[0]+4096)/patchSize);
	texCoords.push_back((base[1]-2048)/patchSize);

	vertices.push_back(base[0]);
	vertices.push_back(base[1]);
	vertices.push_back(z);
	texCoords.push_back((base[0])/patchSize);
	texCoords.push_back((base[1])/patchSize);

	vertices.push_back(base[0]);
	vertices.push_back(base[1]);
	vertices.push_back(z);
	texCoords.push_back((base[0])/patchSize);
	texCoords.push_back((base[1])/patchSize);

	vertices.push_back(base[0]+4096);
	vertices.push_back(base[1]-2048);
	vertices.push_back(z);
	texCoords.push_back((base[0]+4096)/patchSize);
	texCoords.push_back((base[1]-2048)/patchSize);

	vertices.push_back(base[0]+4096);
	vertices.push_back(base[1]);
	vertices.push_back(z);
	texCoords.push_back((base[0]+4096)/patchSize);
	texCoords.push_back((base[1])/patchSize);

	// northern patch
	vertices.push_back(base[0]);
	vertices.push_back(base[1]+4096);
	vertices.push_back(z);
	texCoords.push_back((base[0])/patchSize);
	texCoords.push_back((base[1]+4096)/patchSize);

	vertices.push_back(base[0]+4096);
	vertices.push_back(base[1]+4096);
	vertices.push_back(z);
	texCoords.push_back((base[0]+4096)/patchSize);
	texCoords.push_back((base[1]+4096)/patchSize);

	vertices.push_back(base[0]);
	vertices.push_back(base[1]+4096+2048);
	vertices.push_back(z);
	texCoords.push_back((base[0])/patchSize);
	texCoords.push_back((base[1]+4096+2048)/patchSize);

	vertices.push_back(base[0]);
	vertices.push_back(base[1]+4096+2048);
	vertices.push_back(z);
	texCoords.push_back((base[0])/patchSize);
	texCoords.push_back((base[1]+4096+2048)/patchSize);

	vertices.push_back(base[0]+4096);
	vertices.push_back(base[1]+4096);
	vertices.push_back(z);
	texCoords.push_back((base[0]+4096)/patchSize);
	texCoords.push_back((base[1]+4096)/patchSize);

	vertices.push_back(base[0]+4096);
	vertices.push_back(base[1]+4096+2048);
	vertices.push_back(z);
	texCoords.push_back((base[0]+4096)/patchSize);
	texCoords.push_back((base[1]+4096+2048)/patchSize);

	vertices.insert(vertices.end(), texCoords.begin(), texCoords.end());

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(GLfloat),
	             &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
