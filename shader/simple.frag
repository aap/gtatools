#version 120

varying vec4 v_Color;
varying vec2 v_TexCoord;
uniform sampler2D u_Texture;
uniform int u_TextureType;

uniform vec4 u_Col1;
uniform vec4 u_Col2;

void main(void) {
	vec4 texCol = texture2D(u_Texture, v_TexCoord);
	vec4 fragCol = v_Color;
	if(u_TextureType == 1)	/* alpha texture */
		fragCol.a = texCol.r;
	else
		fragCol *= texCol;

	float alpha = fragCol[3];
	gl_FragColor = fragCol*(u_Col1 + u_Col2*u_Col2.w)*2.0;
	gl_FragColor[3] = alpha; 
}
