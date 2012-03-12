#version 120

attribute vec3 in_Vertex;
attribute vec3 in_Normal;
attribute vec4 in_Color;
attribute vec2 in_TexCoord;

uniform mat4 u_Projection;
uniform mat4 u_ModelView;
uniform mat3 u_NormalMat;
uniform vec4 u_LightPos;
uniform vec3 u_LightCol;
uniform vec3 u_AmbientLight;
uniform vec4 u_MatColor;

varying vec4 v_Color;
varying vec2 v_TexCoord;

void main(void)
{
	vec4 V = u_ModelView * vec4(in_Vertex, 1.0);

	if (in_Normal == vec3(0.0, 0.0, 0.0)) {
		v_Color = (vec4(u_AmbientLight,0.0) + u_MatColor*in_Color) *
		          u_MatColor;
	} else {
		vec3 N = normalize(u_NormalMat * in_Normal);
		float L = max(0.0, dot(N, normalize(u_LightPos.xyz - V.xyz)));
		v_Color = (vec4(u_AmbientLight, 0.0) +
		           in_Color *
		           vec4(u_LightCol,1.0) * vec4(L, L, L, 1.0)) *
		          u_MatColor;
	}

	gl_Position = u_Projection * V;
	v_TexCoord = in_TexCoord;
}
