#include "gta.h"
#include "gl.h"
#include "water.h"

using namespace std;
using namespace gl;

Water water;

void Water::draw(void)
{
	glUniform4fv(u_MatColor, 1, color);

	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);

	glVertexAttrib4f(in_Color, 1.0f, 1.0f, 1.0f, 1.0f);
	glVertexAttrib3f(in_Normal, 0.0f, 0.0f, 0.0f);
	glBindTexture(GL_TEXTURE_2D, tex);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(in_Vertex);
	glEnableVertexAttribArray(in_TexCoord);
	glVertexAttribPointer(in_Vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(in_TexCoord, 2, GL_FLOAT, GL_FALSE, 0,
	                      (GLvoid *) (vertices.size()*3/5*sizeof(GLfloat)));
	glDrawArrays(GL_TRIANGLES, 0, vertices.size()*3/5/3);
	glDisableVertexAttribArray(in_Vertex);
	glDisableVertexAttribArray(in_TexCoord);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Water::loadWaterpro(std::ifstream &f)
{
	float palette[48];
	unsigned char indiceshi[128*128];
	unsigned char indiceslo[64*64];

	f.seekg(4, ios::cur);
	f.read((char*) &palette, 48*sizeof(float));
	f.seekg(0x300, ios::cur);
	f.read((char*) &indiceslo, 64*64);
	f.read((char*) &indiceshi, 128*128);

	float patchSize = 32;
	float base[2];
	TexDictionary *txd = texMan.get("particle.txd");
	if (game == GTA3) {
		base[0] = -2048;
		base[1] = -2048;
		Texture *t = txd->get("water_old");
		cout << t->hasAlpha << endl;
		tex = t->tex;
		color[0] = 1.0f;
		color[1] = 1.0f;
		color[2] = 1.0f;
		color[3] = 1.0f;
	} else {	// must be GTAVC
		base[0] = -2448;
		base[1] = -2048;
		Texture *t = txd->get("waterclear256");
		cout << t->hasAlpha << endl;
		tex = t->tex;
		color[0] = 0.2627f;
		color[1] = 0.2627f;
		color[2] = 0.3921f;
		color[3] = 0.7f;
	}

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

				vertices.push_back(x0+patchSize);
				vertices.push_back(y0);
				vertices.push_back(z);

				vertices.push_back(x0);
				vertices.push_back(y0+patchSize);
				vertices.push_back(z);

				vertices.push_back(x0+patchSize);
				vertices.push_back(y0+patchSize);
				vertices.push_back(z);
			}
		}
	}
	for (uint x = 0; x < 128; x++) {
		for (uint y = 0; y < 128; y++) {
			if (indiceshi[x*128+y] != 0x80) {
				vertices.push_back(0.0f);
				vertices.push_back(0.0f);

				vertices.push_back(1.0f);
				vertices.push_back(0.0f);

				vertices.push_back(0.0f);
				vertices.push_back(1.0f);

				vertices.push_back(1.0f);
				vertices.push_back(0.0f);

				vertices.push_back(0.0f);
				vertices.push_back(1.0f);

				vertices.push_back(1.0f);
				vertices.push_back(1.0f);
			}
		}
	}


	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(GLfloat),
	             &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
