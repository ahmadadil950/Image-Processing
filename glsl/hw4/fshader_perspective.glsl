#version 330

in	vec2	v_TexCoord;			// varying variable for passing texture coordinate from vertex shader

uniform float		u_Perspective[9];
uniform highp float	u_StepX;	// pixel size in x dir
uniform highp float	u_StepY;	// pixel size in y dir
uniform highp float	u_Xscale;	
uniform highp float	u_Yscale;	
uniform highp float	u_Xmin;	
uniform highp float	u_Ymin;	
uniform	sampler2D	u_Sampler;	// uniform variable for the texture image

out	vec4	  fragColor;

void main() 
{
	vec2 tc = v_TexCoord;
	float xx = tc.x / u_StepX; // convert normalized texture position to actual pixel on the image
	float yy = (1-tc.y) / u_StepY;
	xx = xx / u_Xscale;
	yy = yy / u_Yscale;
	xx += u_Xmin;
	yy += u_Ymin;
	
	float w =  u_Perspective[2]*xx + u_Perspective[5]*yy + u_Perspective[8];
	float x = (u_Perspective[0]*xx + u_Perspective[3]*yy + u_Perspective[6])/w;
	float y = (u_Perspective[1]*xx + u_Perspective[4]*yy + u_Perspective[7])/w;
	x = x * u_StepX;
	y = 1 - (y * u_StepY);

	if (x<0 || x>1 || y<0 || y>1)
		fragColor = vec4(0, 0, 0, 1.0);
	else
		fragColor = vec4(texture(u_Sampler, vec2(x, y)).rgb, 1.0);
}