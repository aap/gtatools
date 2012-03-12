#version 120

attribute vec3 in_Vertex;
attribute vec4 in_Color;
attribute vec2 in_TexCoord;

uniform mat4 u_Projection;
uniform mat4 u_ModelView;
uniform vec4 u_LightPos;

varying vec4 v_Color;
varying vec2 v_TexCoord;

void main(void)
{
	gl_Position = u_Projection * u_ModelView * vec4(in_Vertex, 1.0);
	v_Color = in_Color;
	v_TexCoord = in_TexCoord;
}
