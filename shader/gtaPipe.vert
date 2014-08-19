#version 120

attribute vec3 in_Vertex;
attribute vec3 in_Normal;
attribute vec4 in_Color;
attribute vec2 in_TexCoord;

uniform mat4 u_Projection;
uniform mat4 u_ModelView;
uniform mat3 u_NormalMat;
//uniform vec4 u_LightPos;
uniform vec3 u_LightCol;
uniform vec3 u_LightDir;
uniform vec3 u_AmbientLight;
uniform vec4 u_MatColor;

varying vec4 v_Color;
varying vec2 v_TexCoord;
varying float eyez;
varying float logz;

void main(void)
{
	vec4 V = u_ModelView * vec4(in_Vertex, 1.0);
	eyez = -V.z;

	vec4 lightVal = vec4(0.0f, 0.0f, 0.0f, 0.0f);
/*
	if (in_Normal != vec3(0.0, 0.0, 0.0)) {
//		vec3 N = normalize(u_NormalMat * in_Normal);
//		float L = max(0.0, dot(N, normalize(u_LightPos.xyz - V.xyz)));
		vec3 N = u_NormalMat * in_Normal;
		float L = max(0.0, dot(N, u_LightDir));
		lightVal = vec4(u_LightCol,0.0) * vec4(L, L, L, 0.0);
	}
*/
/*
	v_Color = clamp(vec4(u_AmbientLight, 0.0) +
	                in_Color + lightVal, 0.0, 1.0);
	v_Color *= u_MatColor;
*/
	v_Color = clamp(vec4(u_AmbientLight, 0.0)*u_MatColor + in_Color, 0.0, 1.0);
	v_Color.a *= u_MatColor.a;

	gl_Position = u_Projection * V;
	v_TexCoord = in_TexCoord;

/*
	float C = 0.01;
	float far = u_Fog.end;
	float near = 1.0;
	logz = log(gl_Position.w*C + 1)/log(far*C + 1);
	gl_Position.z = (2*logz - 1)*gl_Position.w;
*/
}
