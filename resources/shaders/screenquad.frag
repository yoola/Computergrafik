#version 150
#extension GL_ARB_explicit_attrib_location : require

uniform sampler2D ColorTex;
uniform vec2 resoulotion;
uniform bool Luminance;
uniform bool HorizontalMirror;
uniform bool VerticalMirror;
uniform bool Gaussian;
in vec2 pass_TexCoord;

out vec4 out_Color;

void main(void)
{
	vec2 texCoord = pass_TexCoord;



	if (HorizontalMirror) {

		texCoord = vec2(texCoord.x, 1.0f - texCoord.y);
		out_Color = texture(ColorTex, vec2(texCoord.x,texCoord.y));
	}

	if (VerticalMirror) {

		texCoord = vec2(1.0f - texCoord.x, texCoord.y);
		out_Color = texture(ColorTex, vec2(texCoord.x,texCoord.y));
	}


	if(Gaussian){

	    vec2 swap_res = vec2(1.0) / resoulotion;
        for (int x = -1; x <= 1; ++x)
        {
            for (int y = -1; y <= 1; ++y)
            {
                float d = abs(x) + abs(y);
                float factor = 0.25f * pow(2.0f, -d);
                vec2 offset = vec2(float(x), float(y));
                vec2 texCrd = texCoord + offset * swap_res;
                vec4 cl = texture(ColorTex, texCrd);
                out_Color += factor * cl;
            }
        }

		// computing the weighted sum

		}else{

		out_Color = texture(ColorTex, texCoord);
		}
		
	if(Luminance){

		float luminance = 0.2126 *out_Color.r + 0.7152*out_Color.g  + 0.0722*out_Color.b;

		out_Color = vec4(luminance,luminance, luminance, 1.0f);
	}
}
