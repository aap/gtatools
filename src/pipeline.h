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

/* vertex */
extern GLint in_Vertex;
extern GLint in_Normal;
extern GLint in_Color;
extern GLint in_TexCoord;

extern GLint u_Projection;
extern GLint u_ModelView;
extern GLint u_NormalMat;
extern GLint u_LightPos;
extern GLint u_LightCol;
extern GLint u_AmbientLight;
extern GLint u_MatColor;

/* fragment */
extern GLint u_Texture;
}


#endif
