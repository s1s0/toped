// Fragment Shader – file "minimal.frag"

#version 330

uniform vec3  in_Color;
uniform float in_Alpha;
uniform uint  in_Stipple[32];
uniform bool  in_StippleEn = false;

//in  vec4 gl_FragCoord;
out vec4 out_Color;

void main(void)
{
   bool dropThePixel = false;
   if (in_StippleEn)
   {
      uvec2 ufCoord = uvec2(gl_FragCoord.x, gl_FragCoord.y);
      uint index = ufCoord.y % 32;
      uint mask  = uint(1) << (ufCoord.x % 32);
      dropThePixel = !bool(in_Stipple[index] & mask);
   }
   if (dropThePixel)
      discard;
   else
      out_Color = vec4(in_Color,in_Alpha);
}
