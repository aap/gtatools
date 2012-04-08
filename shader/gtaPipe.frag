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

vec4 overlay(vec4 c1, vec4 c2)
{
	return c1*(c1+2*c2*(vec4(1.0)-c1));
}

vec4 screen(vec4 c1, vec4 c2)
{
	return vec4(1.0) - (vec4(1.0)-c2)*(vec4(1.0)-c1);
}

void main(void) {
	const float LOG2 = 1.442695;
	float z = gl_FragCoord.z / gl_FragCoord.w;
	float fogFactor = 1.0;
	if (u_Fog.density == 0.0) {
		fogFactor = (u_Fog.end - z)/(u_Fog.end - u_Fog.start);
	} else if (u_Fog.density > 0.0) {
		fogFactor = exp2(-u_Fog.density*u_Fog.density * 
				       z*z * LOG2);
	}
	fogFactor = clamp(fogFactor, 0.0, 1.0);

	vec4 blur = vec4(0.21, 0.14, 0.06, 0.0);

	vec4 fragCol = v_Color * texture2D(u_Texture, v_TexCoord);
	fragCol = mix(u_Fog.color, fragCol, fogFactor);
	gl_FragColor = fragCol;
//	gl_FragColor = overlay(gl_FragColor, fragCol);
//	blur = blur*blur;
//	gl_FragColor += blur;
}
