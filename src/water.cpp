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

void Water::loadWater(std::ifstream &f)
{
	string line;
	getline(f, line);
	if (line != "processed") {
		cout << "unknown format of SA water.dat\n";
		return;
	}
	color[0] = 90.0f/255.0f;
	color[1] = 170.0f/255.0f;
	color[2] = 170.0f/255.0f;
	color[3] = 240.0f/255.0f;

	TexDictionary *txd = texMan.get("particle.txd");
	Texture *t = txd->get("waterclear256");
	tex = t->tex;

	vector<GLfloat> texCoords;
	GLfloat x[4], y[4], z[4], f1[4], f2[4], f3[4], f4[4];
	GLuint param;
	vector<string> fields;
	while (!f.eof()) {
		getline(f, line);
		getFields(line, ' ', fields);

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
		tex = t->tex;
		color[0] = 1.0f;
		color[1] = 1.0f;
		color[2] = 1.0f;
		color[3] = 1.0f;
	} else {	// must be GTAVC
		base[0] = -2448;
		base[1] = -2048;
		Texture *t = txd->get("waterclear256");
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
