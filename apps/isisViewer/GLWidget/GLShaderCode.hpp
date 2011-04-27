#define STRINGIFY(A) #A

std::string colormap_shader_code = STRINGIFY(
									   uniform sampler3D imageTexture;
									   uniform sampler1D lut;
									   uniform float min;
									   uniform float max;
									   uniform float upper_threshold;
									   uniform float lower_threshold;
									   uniform float bias;
									   uniform float scaling;
									   uniform float opacity;
									   uniform float killZeros;
									   void main ()
{
	float range = max - min;
	float i = texture3D( imageTexture, gl_TexCoord[0].xyz ).r;
	vec4 colorLut = texture1D( lut, i );
	colorLut.a = opacity;
	float inormed = ( i * range ) + min;
	if( inormed > upper_threshold || inormed < lower_threshold )  {
		colorLut.a = 0;
	}
	if( killZeros == 1 && ( inormed > -0.004 && inormed < 0.004 ) ) {
		colorLut.a = 0;
	}
	gl_FragColor = ( colorLut + bias / range ) * scaling;
}
								   );

std::string scaling_shader_code = STRINGIFY(
									  uniform sampler3D imageTexture;
									  uniform float min;
									  uniform float max;
									  uniform float upper_threshold;
									  uniform float lower_threshold;
									  uniform float scaling;
									  uniform float bias;
									  uniform float opacity;
									  void main()
{
	float range = max - min;
	vec4 color = texture3D( imageTexture, gl_TexCoord[0].xyz );
	color.a = opacity;

	if( ( color.r * range ) + min > upper_threshold || ( color.r * range ) + min < lower_threshold ) {
		color.a = 0;
	}


	gl_FragColor = ( color + bias / range ) * scaling;
}
								  );
