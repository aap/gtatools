#version 120

varying vec4 v_Color;
varying vec2 v_TexCoord;
uniform sampler2D u_Texture;

void main(void) {
	gl_FragColor = v_Color * texture2D(u_Texture, v_TexCoord);
}
