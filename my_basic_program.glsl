#version 330

#ifdef VERTEX_SHADER

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 texcoords;

uniform mat4 mvp_matrix;

void main()
{
  gl_Position = mvp_matrix * vec4(position, 1);
}

#endif

#ifdef FRAGMENT_SHADER

out vec4 fragment_color;

void main()
{
  fragment_color.rgb = vec3(gl_FragCoord.z, 0, 0);
}

#endif
