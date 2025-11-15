#include "common.glsl"

in vec3 vertPos;
in vec3 vertPosition_World;
in vec3 vertNormal;
in vec4 vertColor;
in vec2 vertTexcoord;
in vec3 vertTangent;

out vec4 FragColor;

void main()
{

	if (u_EnablePrimitiveColor!=0)
	{
		if (u_EnableTexture != 0)
			FragColor = vertColor * texture(u_TextureUnit, vertTexcoord);
		else
			FragColor = vertColor;
	}
	else
	{
		if (u_EnableLighting != 0)
		{
			FragColor=vec4(u_Material.AmbientColor,1);
			FragColor=vertColor;
//			FragColor=vec4(0,1,0,1);
//			return;
			//vec4(Ads(0,vec4(vertPos,1),vertNormal),1);
			if (u_EnableTexture != 0)
			{
				vec4 texc = texture(u_TextureUnit, vertTexcoord);
				FragColor = calculateLighting(vertPos, vertNormal, texc);
			}
			else
			{
				FragColor = calculateLighting(vertPos, vertNormal, vec4(0.0));
			}
		}
		else
		{
			if (u_EnableTexture != 0)
				FragColor = texture(u_TextureUnit, vertTexcoord);
			else
				FragColor = vec4(vertNormal,1);
		}
	}
}
