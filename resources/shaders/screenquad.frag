#version 150
#extension GL_ARB_explicit_attrib_location : require

uniform sampler2D ColorTex;

uniform bool Luminance;
uniform bool HorizontalMirror;
uniform bool VerticalMirror;
uniform bool Gaussian;

in vec2 pass_TexCoord;

out vec4 out_Color;

void main(void)
{	
	vec2 texCoord = pass_TexCoord;
	out_Color = texture(ColorTex, texCoord);

	if(Luminance){

		float luminance = 0.2126 *out_Color.r + 0.7152*out_Color.g  + 0.0722*out_Color.b;

		out_Color = vec4(luminance,luminance, luminance, 1.0f);
	}

	if (HorizontalMirror) {

		texCoord = vec2(texCoord.x, 1.0f - texCoord.y);
		out_Color = texture(ColorTex, vec2(texCoord.x,texCoord.y));
	}

	if (VerticalMirror) {

		texCoord = vec2(1.0f - texCoord.x, texCoord.y);
		out_Color = texture(ColorTex, vec2(texCoord.x,texCoord.y));
	}
	
	
	if(Gaussian){

	    vec2 pix_neighbours[9];
	    vec2 pixel_size = texCoord / gl_FragCoord.xy;

	    // calculating offset to middle pixel
	    pix_neighbours[0] = vec2(texCoord.x - pixel_size.x, texCoord.y + pixel_size.y );
	    pix_neighbours[1] = vec2(texCoord.x           	 ,  texCoord.y + pixel_size.y );
	    pix_neighbours[2] = vec2(texCoord.x + pixel_size.x, texCoord.y + pixel_size.y );
	    pix_neighbours[3] = vec2(texCoord.x - pixel_size.x, texCoord.y                );
	    pix_neighbours[4] = vec2(texCoord.x               , texCoord.y                );
	    pix_neighbours[5] = vec2(texCoord.x + pixel_size.x, texCoord.y                );
	    pix_neighbours[6] = vec2(texCoord.x - pixel_size.x, texCoord.y - pixel_size.y );
	    pix_neighbours[7] = vec2(texCoord.x               , texCoord.y - pixel_size.y );
		pix_neighbours[8] = vec2(texCoord.x + pixel_size.x, texCoord.y - pixel_size.y );

		// weighting the neighbouring pixels
		vec4 p0 = 1/16 * texture(ColorTex, pix_neighbours[0]);
		vec4 p1 = 1/8  * texture(ColorTex, pix_neighbours[1]);
		vec4 p2 = 1/16 * texture(ColorTex, pix_neighbours[2]);
		vec4 p3 = 1/8  * texture(ColorTex, pix_neighbours[3]);
		vec4 p4 = 1/4  * texture(ColorTex, pix_neighbours[4]);
		vec4 p5 = 1/8  * texture(ColorTex, pix_neighbours[5]);
		vec4 p6 = 1/16 * texture(ColorTex, pix_neighbours[6]);
		vec4 p7 = 1/8  * texture(ColorTex, pix_neighbours[7]);
		vec4 p8 = 1/16 * texture(ColorTex, pix_neighbours[8]);

		// computing the weighted sum
		out_Color = p0 + p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8;
	}
}