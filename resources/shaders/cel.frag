#version 150

in  vec3 pass_Normal;
in  vec3 pass_Color;
in  vec3 pass_LightVector;

out vec4 out_Color;

void main(void)
{

  // resource: http://www.informatik.uni-oldenburg.de/~trigger/content/opengl/opengl_course/slides/2010-JOGL-11-Toon-Shading.pdf
  
  vec3 normal = normalize(pass_Normal);
  float intensity = dot(normalize(pass_LightVector.xyz), normal);


  if(intensity > 0.95)
        out_Color = vec4(0.5, 1.0, 0.5, 1.0) + vec4(pass_Color, 1.0f);
    else if(intensity > 0.5)
        out_Color = vec4(0.3, 0.6, 0.3, 1.0) + vec4(pass_Color, 1.0f);
    else if(intensity > 0.25)
        out_Color = vec4(0.2, 0.4, 0.2, 1.0) + vec4(pass_Color, 1.0f);
    else
        out_Color = vec4(0.1, 0.2, 0.1, 1.0) + vec4(pass_Color, 1.0f);    
}