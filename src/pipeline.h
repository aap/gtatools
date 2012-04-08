#ifndef PIPELINE_H
#define PIPELINE_H
#include <GL/glew.h>

namespace gl {

class Pipeline {
public:
	void use(void);
	void getVar(const char *name, GLint *var, GLint type);
	void load(const char *vertsrc, const char *fragsrc);
	void printLog(GLuint object);

	GLint program;
	GLint vertshader;
	GLint fragshader;

};

extern GLint in_Vertex;
extern GLint in_Normal;
extern GLint in_Color;
extern GLint in_TexCoord;

class State {
public:
	/* vertex */
	GLint u_Projection;
	glm::mat4 projection;
	GLint u_ModelView;
	glm::mat4 modelView;
	GLint u_NormalMat;
	glm::mat3 normalMat;
	GLint u_LightPos;
	glm::vec4 lightPos;
	GLint u_LightCol;
	glm::vec3 lightCol;
	GLint u_AmbientLight;
	glm::vec3 ambientLight;
	GLint u_MatColor;
	glm::vec4 matColor;

	/* fragment */
	GLint u_Texture;
	GLint texture;
	GLint u_FogColor;
	glm::vec4 fogColor;
	GLint u_FogStart;
	GLfloat fogStart;
	GLint u_FogEnd;
	GLfloat fogEnd;
	GLint u_FogDensity;
	GLfloat fogDensity;

	void calculateNormalMat(void);
	void updateMatrices(void);
	void updateMaterial(void);
	void updateLight(void);
	void updateTexture(void);
	void updateFog(void);
	void updateAll(void);
};

extern State state;

};

#endif
