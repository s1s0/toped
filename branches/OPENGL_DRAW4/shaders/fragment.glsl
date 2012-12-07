// Fragment Shader � file "minimal.frag"

#version 330

uniform vec3  in_Color;
uniform float in_Alpha;
uniform uint  in_Stipple[33];
uniform bool  in_StippleEn = false;

//in  vec4 gl_FragCoord;
out vec4 out_Color;

void main(void)
{
   bool dropThePixel = false;
   if (in_StippleEn)
   {
      uvec2 ufCoord = uvec2(gl_FragCoord.x, gl_FragCoord.y);
      uint index = 31u - (ufCoord.y % 32u);
      uint mask  = uint(0x80000000) >> (ufCoord.x % 32u);
      dropThePixel = !bool(in_Stipple[index+1] & mask);
   }
   if (dropThePixel)
      discard;
   else
      out_Color = vec4(in_Color,in_Alpha);
}
