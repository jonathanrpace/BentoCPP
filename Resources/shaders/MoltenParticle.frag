#version 330 core

// Outputs
layout( location = 0 ) out vec4 out_fragColor;


out float gl_FragDepth ;

in Varying
{
	vec4 in_color;
};

void main(void)
{
	if ( in_color.a <= 0.0001 )
	{
		discard;
	}

	gl_FragDepth = in_color.x;

	out_fragColor = in_color;
} 