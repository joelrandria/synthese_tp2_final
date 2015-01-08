#version 330

#ifdef VERTEX_SHADER

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 texcoords;

uniform mat4 mvp_matrix;
uniform mat4 mv_normalmatrix;

out vec3 fragment_position;
out vec3 fragment_normal;
out vec2 fragment_texcoords;

void main()
{
  gl_Position = mvp_matrix * vec4(position, 1);

  fragment_position = position;
  fragment_normal = (mv_normalmatrix * vec4(normal, 0)).xyz;
  fragment_texcoords = texcoords.xy;
}

#endif

#ifdef FRAGMENT_SHADER

in vec3 fragment_position;
in vec3 fragment_normal;
in vec2 fragment_texcoords;

uniform sampler2D diffuse_texture;

out vec4 fragment_color;

void main()
{
  vec3 n = normalize(fragment_normal);

  fragment_color.rgb = texture(diffuse_texture, fragment_texcoords).rgb;

  //fragment_color.rgb = vec3(fragment_texcoords, 0);
}

#endif
