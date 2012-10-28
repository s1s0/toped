// Vertex Shader � file "minimal.vert"

#version 330

layout (location = 0) in      vec2  in_Vertex;
uniform mat4 in_CTM;
uniform float in_Z = 0;

void main(void)
{
   gl_Position = in_CTM * vec4(in_Vertex, in_Z, 1.0);
}