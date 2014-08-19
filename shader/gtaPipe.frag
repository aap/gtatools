#version 120

struct fogParams
{
	vec4 color;
	float start;
	float end;
	float density;	// if density == 0 => linear, if < 0 no fog, else exp2
};
 
uniform fogParams u_Fog;

varying vec4 v_Color;
varying vec2 v_TexCoord;
varying float eyez;
varying float logz;
uniform sampler2D u_Texture;

uniform vec4 u_Col1;
uniform vec4 u_Col2;

void
main(void)
{
	float z = 1 / gl_FragCoord.w;
	float fogFactor = (u_Fog.end - z)/(u_Fog.end - u_Fog.start);
	fogFactor = clamp(fogFactor, 0.0, 1.0);

	vec4 fragCol = v_Color * texture2D(u_Texture, v_TexCoord);
	float alpha = fragCol[3];
	fragCol = mix(u_Fog.color, fragCol, fogFactor);

	gl_FragColor = fragCol*(u_Col1 + u_Col2*u_Col2.w)*2.0;
	gl_FragColor[3] = alpha;
}
