#include <iostream>
#include <GL/glew.h>

#include "gta.h"
#include "gl.h"
#include "renderer.h"
#include "console.h"

using namespace std;

Console *console;

#include "font.h"
#define BORDER 10.f

void Console::init(void)
{
	GLfloat vertices[] = {
		8.0, 512.0, 0.0,
		8.0, 0.0, 0.0,
		16.0, 512.0, 0.0,
		8.0, 0.0, 0.0,
		16.0, 512.0, 0.0,
		16.0, 0.0, 0.0,

		0.0, 0.25,
		0.0, 0.5,
		1.0, 0.25,
		0.0, 0.5,
		1.0, 0.25,
		1.0, 0.5,
	};

	GLfloat bgverts[] = {
		BORDER, height-BORDER, 0.0,
		width-BORDER, height-BORDER, 0.0,
		BORDER, BORDER, 0.0,
		width-BORDER, height-BORDER, 0.0,
		BORDER, BORDER, 0.0,
		width-BORDER, BORDER, 0.0
	};

	THREADCHECK();

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 8, 16*128, 0, GL_ALPHA, GL_UNSIGNED_BYTE, font);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 8, 16*128, 0, GL_RED, GL_UNSIGNED_BYTE, font);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
		&vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &bgvbo);
	glBindBuffer(GL_ARRAY_BUFFER, bgvbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bgverts),
		&bgverts[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Console::draw(void)
{
	THREADCHECK();

	glDisable(GL_DEPTH_TEST);

	glVertexAttrib4f(gl::in_Color, 0.0f, 0.0f, 0.0f, 0.6f);
	glBindTexture(GL_TEXTURE_2D, renderer->whiteTex);
	glBindBuffer(GL_ARRAY_BUFFER, bgvbo);
	glEnableVertexAttribArray(gl::in_Vertex);
	glVertexAttribPointer(gl::in_Vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(gl::in_Vertex);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	gl::state.textureType = 1;
	gl::state.updateTexture();
	glVertexAttrib4f(gl::in_Color, 1.0f, 1.0f, 1.0f, 1.0f);
//	glBindTexture(GL_TEXTURE_2D, renderer->whiteTex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(gl::in_Vertex);
	glEnableVertexAttribArray(gl::in_TexCoord);
	glVertexAttribPointer(gl::in_Vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(gl::in_TexCoord, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(6*3*sizeof(GLfloat)));
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(gl::in_Vertex);
	glDisableVertexAttribArray(gl::in_TexCoord);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Console::setDimensions(int w, int h)
{
	width = w;
	height = h;

	GLfloat bgverts[] = {
		BORDER, height-BORDER, 0.0,
		width-BORDER, height-BORDER, 0.0,
		BORDER, BORDER, 0.0,
		width-BORDER, height-BORDER, 0.0,
		BORDER, BORDER, 0.0,
		width-BORDER, BORDER, 0.0
	};

	glBindBuffer(GL_ARRAY_BUFFER, bgvbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bgverts),
		&bgverts[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

