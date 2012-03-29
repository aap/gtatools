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
uniform sampler2D u_Texture;

void main(void) {
	const float LOG2 = 1.442695;
	float z = gl_FragCoord.z / gl_FragCoord.w;
	float fogFactor = 0.0;
	if (u_Fog.density == 0.0) {
		fogFactor = (u_Fog.end - z)/(u_Fog.end - u_Fog.start);
	} else if (u_Fog.density > 0.0) {
		fogFactor = exp2(-u_Fog.density*u_Fog.density * 
				       z*z * LOG2);
	}
	fogFactor = clamp(fogFactor, 0.0, 1.0);

	vec4 fragCol = v_Color * texture2D(u_Texture, v_TexCoord);
	gl_FragColor = mix(u_Fog.color, fragCol, fogFactor);
}
