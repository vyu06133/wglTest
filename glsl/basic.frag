#include "common.glsl"

in vec3 v_pos;
in vec3 v_position_World;
in vec3 v_normal;
in vec4 v_color;
in vec2 v_texcoord;
in vec3 v_tangent;

out vec4 FragColor;

void main()
{

	if (u_EnablePrimitiveColor!=0)
	{
		if (u_EnableTexture != 0)
			FragColor = v_color * texture(u_TextureUnit, v_texcoord);
		else
			FragColor = v_color;
	}
	else if (u_EnableLighting != 0)
	{
		FragColor=vec4(u_Material.AmbientColor,1);
		FragColor=v_color;
//			FragColor=vec4(0,1,0,1);
//			return;
		//vec4(Ads(0,vec4(v_Pos,1),v_Normal),1);
		if (u_EnableTexture != 0)
		{
			vec4 texc = texture(u_TextureUnit, v_texcoord);
			FragColor = calculateLighting(v_pos, v_normal, texc);
		}
		else
		{
			FragColor = calculateLighting(v_pos, v_normal, vec4(0.0));
		}
	}
	else
	{
		if (u_EnableTexture != 0)
			FragColor = texture(u_TextureUnit, v_texcoord);
		else
		{
			FragColor = vec4(v_normal,1);
			FragColor = v_color;
		}
	}
	if(u_DebugFragColor != 0)
		FragColor = vec4(1,0,0,1);
FragColor+=v_color*0.000001+vec4(v_tangent*0.000001,0);
}
