#include <cstdio>

#include "gta.h"
#include "gl.h"

using namespace std;

namespace gl {

/* shader variables */
/* vertex */
GLint in_Vertex;
GLint in_Normal;
GLint in_Color;
GLint in_TexCoord;

GLint u_Projection;
GLint u_ModelView;
GLint u_NormalMat;
GLint u_LightPos;
GLint u_LightCol;
GLint u_AmbientLight;
GLint u_MatColor;
/* fragment */
GLint u_Texture;

void Pipeline::getVar(const char *name, GLint *var, GLint type)
{
	GLint v;

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
	currentPipe = this;
	glUseProgram(currentPipe->program);

	getVar("in_Vertex", &in_Vertex, 0);
	getVar("in_Normal", &in_Normal, 0);
	getVar("in_Color", &in_Color, 0);
	getVar("in_TexCoord", &in_TexCoord, 0);

	getVar("u_Projection", &u_Projection, 1);
	getVar("u_ModelView", &u_ModelView, 1);
	getVar("u_NormalMat", &u_NormalMat, 1);
	getVar("u_LightPos", &u_LightPos, 1);
	getVar("u_LightCol", &u_LightCol, 1);
	getVar("u_AmbientLight", &u_AmbientLight, 1);
	getVar("u_MatColor", &u_MatColor, 1);

	getVar("u_Texture", &u_Texture, 1);
}

void Pipeline::load(const char *vertsrc, const char *fragsrc)
{
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
