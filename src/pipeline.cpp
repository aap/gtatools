#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "gl.h"
#include "pipeline.h"

using namespace std;

namespace gl {

State state;

/* shader variables */
GLint in_Vertex;
GLint in_Normal;
GLint in_Color;
GLint in_TexCoord;

void State::calculateNormalMat(void)
{
	normalMat = glm::inverseTranspose(glm::mat3(modelView));
}

void State::updateMatrices(void)
{
	THREADCHECK();
	glUniformMatrix4fv(u_Projection, 1, GL_FALSE,
	                   glm::value_ptr(projection));
	glUniformMatrix4fv(u_ModelView, 1, GL_FALSE,
	                   glm::value_ptr(modelView));
	glUniformMatrix3fv(u_NormalMat, 1, GL_FALSE,
	                   glm::value_ptr(normalMat));
}

void State::updateMaterial(void)
{
	THREADCHECK();
	glUniform4fv(u_MatColor, 1, glm::value_ptr(matColor));
}

void State::updateLight(void)
{
	THREADCHECK();
	glUniform4fv(u_LightPos, 1, glm::value_ptr(lightPos));
	glUniform3fv(u_LightCol, 1, glm::value_ptr(lightCol));
	glUniform3fv(u_AmbientLight, 1, glm::value_ptr(ambientLight));
}

void State::updateTexture(void)
{
	THREADCHECK();
	glUniform1i(u_Texture, texture);
}

void State::updateFog(void)
{
	THREADCHECK();
	glUniform4fv(u_FogColor, 1, glm::value_ptr(fogColor));
	glUniform1f(u_FogDensity, fogDensity);
	glUniform1f(u_FogStart, fogStart);
	glUniform1f(u_FogEnd, fogEnd);
}

void State::updateAll(void)
{
	updateMatrices();
	updateMaterial();
	updateLight();
	updateTexture();
	updateFog();
}

void Pipeline::getVar(const char *name, GLint *var, GLint type)
{
	THREADCHECK();
	GLint v = -1;

	if (type == 0) {
		v = glGetAttribLocation(currentPipe->program, name);
//		if (v == -1)
//			cerr << "Couldn't find attribute " << name << endl;
	} else if (type == 1) {
		v = glGetUniformLocation(currentPipe->program, name);
//		if (v == -1)
//			cerr << "Couldn't find uniform " << name << endl;
	}
	*var = v;
}

void Pipeline::use(void)
{
	THREADCHECK();
	currentPipe = this;
	glUseProgram(currentPipe->program);

	getVar("in_Vertex", &in_Vertex, 0);
	getVar("in_Normal", &in_Normal, 0);
	getVar("in_Color", &in_Color, 0);
	getVar("in_TexCoord", &in_TexCoord, 0);

	getVar("u_Projection", &state.u_Projection, 1);
	getVar("u_ModelView", &state.u_ModelView, 1);
	getVar("u_NormalMat", &state.u_NormalMat, 1);
	getVar("u_LightPos", &state.u_LightPos, 1);
	getVar("u_LightCol", &state.u_LightCol, 1);
	getVar("u_AmbientLight", &state.u_AmbientLight, 1);
	getVar("u_MatColor", &state.u_MatColor, 1);

	getVar("u_Texture", &state.u_Texture, 1);
	getVar("u_Fog.color", &state.u_FogColor, 1);
	getVar("u_Fog.start", &state.u_FogStart, 1);
	getVar("u_Fog.end", &state.u_FogEnd, 1);
	getVar("u_Fog.density", &state.u_FogDensity, 1);
}

void Pipeline::load(const char *vertsrc, const char *fragsrc)
{
	THREADCHECK();
	ifstream shrSrc;
	GLint filelength;
	GLint success;
	GLchar *fragShaderSrc, *vertShaderSrc;

	/* load shader source */
	shrSrc.open(vertsrc);
	if (shrSrc.fail()) {
		cerr << "couldn't open" << vertsrc << endl;
		exit(1);
	}
	shrSrc.seekg(0, ios::end);
	filelength = shrSrc.tellg();
	shrSrc.seekg(0, ios::beg);
	vertShaderSrc = new char[filelength+1];
	shrSrc.read(vertShaderSrc, filelength);
	vertShaderSrc[filelength] = '\0';
	shrSrc.close();
	
	shrSrc.open(fragsrc);
	if (shrSrc.fail()) {
		cerr << "couldn't open" << fragsrc << endl;
		exit(1);
	}
	shrSrc.seekg(0, ios::end);
	filelength = shrSrc.tellg();
	shrSrc.seekg(0, ios::beg);
	fragShaderSrc = new char[filelength+1];
	shrSrc.read(fragShaderSrc, filelength);
	fragShaderSrc[filelength] = '\0';
	shrSrc.close();
	

	/* compile vertex shader */
	vertshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertshader, 1, (const GLchar **)&vertShaderSrc, NULL);
	delete[] vertShaderSrc;
	glCompileShader(vertshader);
	glGetShaderiv(vertshader, GL_COMPILE_STATUS, &success);
	if (!success) {
		cerr << "Error: " << vertsrc << endl;
		printLog(vertshader);
		exit(1);
	}

	/* compile fragment shader */
	fragshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragshader, 1, (const GLchar **)&fragShaderSrc, NULL);
	delete[] fragShaderSrc;
	glCompileShader(fragshader);
	glGetShaderiv(fragshader, GL_COMPILE_STATUS, &success);
	if (!success) {
		cerr << "Error: " << fragsrc << endl;
		printLog(fragshader);
		exit(1);
	}


	/* link program */
	program = glCreateProgram();
	glAttachShader(program, vertshader);
	glAttachShader(program, fragshader);

	glLinkProgram(program);
	glGetProgramiv(program,GL_LINK_STATUS, &success);
	if (!success) {
		cerr << "Error: linking failed\n";
		printLog(program);
		exit(1);
	}
}

void Pipeline::printLog(GLuint object)
{
	THREADCHECK();
	GLint len;
	char *log;

	if (glIsShader(object)) {
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &len);
		log = new char[len];
		glGetShaderInfoLog(object, len, NULL, log);
	} else if (glIsProgram(object)) {
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &len);
		log = new char[len];
		glGetProgramInfoLog(object, len, NULL, log);
	} else {
		cerr << "printLog: Neither shader nor program\n";
		return;
	}
	cout << log;
	delete[] log;
}

}
