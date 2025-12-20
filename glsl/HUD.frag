#version 450 compatibility
#pragma optimize(off)
in vec4 v_color;
in vec3 v_position;
in vec2 v_tex;

uniform sampler2D u_tex;
uniform vec4 u_fontColor;

out vec4 FragColor;
void main()
{
//	FragColor = vertColor + texture(u_tex, vertTex);
//	FragColor = /*texture(u_tex, vertTex)+*/vertColor;
	vec2 uv = v_tex;
//	vec2 uv = vec2(vertColor.r,vertColor.a);
	vec4 c = u_fontColor;
	float alpha = texture(u_tex, uv).r;
	c *= alpha;
	FragColor = c;//texture(u_tex, uv)+vertColor*0.5f;
//	FragColor = texture(u_tex, vertTex)+vertColor*0.5f;
//	FragColor = texture(u_tex, vertPos.xy);
//	FragColor = vec4(vertTex.xy,0,1);
	FragColor += v_color * 0.0000001;//最適化でアトリビュートが消えないように使っておく
}
