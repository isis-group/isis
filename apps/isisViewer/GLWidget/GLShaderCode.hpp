#define STRINGIFY(A) #A

std::string colormap_shader_code = STRINGIFY(
	uniform sampler3D imageTexture; 
	uniform sampler1D lut; 
	uniform float min;
	uniform float max;
	uniform float upper_threshold;
	uniform float lower_threshold;
	void main () 
	{ 
		float i = texture3D(imageTexture, gl_TexCoord[0].xyz).r;
		vec4 color = texture1D(lut,i);
		if( i > ((1.0/(max-min)) * (upper_threshold-min)) || i < ((1.0/(max-min)) * (lower_threshold-min))) 
		{
			color.a = 0;
		}
		gl_FragColor = color;  
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
		float extent = max - min;
		vec4 color = texture3D(imageTexture, gl_TexCoord[0].xyz); 
		color.a = opacity; 
		if( color.r > ((1.0/(max-min)) * (upper_threshold-min)) || color.r < ((1.0/(max-min)) * (lower_threshold-min))) 
		{
			color.a = 0;
		}
		gl_FragColor = (color + bias/extent) * scaling; 
	}
);
