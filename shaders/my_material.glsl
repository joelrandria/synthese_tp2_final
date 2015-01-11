#version 330

#ifdef VERTEX_SHADER

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 texcoords;

uniform mat4 mv_matrix;
uniform mat4 mvp_matrix;
uniform mat4 mv_normalmatrix;

out vec3 fragment_position;
out vec3 fragment_normal;
out vec2 fragment_texcoords;

void main()
{
  gl_Position = mvp_matrix * vec4(position, 1);

  fragment_position = (mv_matrix * vec4(position, 1)).xyz;
  fragment_normal = (mv_normalmatrix * vec4(normal, 0)).xyz;
  fragment_texcoords = texcoords.xy;
}

#endif

#ifdef FRAGMENT_SHADER

#define POINT_LIGHTS_MAX_COUNT	20

#define PI 3.141592653589793238462643383279

struct point_light
{
  vec4 position;

  vec4 color;
  float constant_attenuation;
  float linear_attenuation;
  float quadratic_attenuation;

  float specularity;
};

in vec3 fragment_position;
in vec3 fragment_normal;
in vec2 fragment_texcoords;

uniform mat4 v_matrix;

uniform int light_count;
uniform point_light_buffer
{
  point_light lights[POINT_LIGHTS_MAX_COUNT];
};

uniform bool material_diffuse_color_enabled;
uniform bool material_diffuse_texture_enabled;

uniform vec3 material_diffuse_color;
uniform sampler2D material_diffuse_texture;

uniform float material_specularity;
uniform float material_specularity_blending;

out vec4 fragment_color;

vec3 incidentLight(int light)
{
  vec3 n;
  vec3 o;
  vec3 q;
  vec3 l;
  vec3 h;

  float d;
  float cos_nl;
  float cos_nh;
  float falloff;

  vec3 diffuse;
  vec3 incident;
  float reflection;

  n = normalize(fragment_normal);
  o = normalize(fragment_position * -1);

  q = (v_matrix * vec4(lights[light].position.xyz, 1)).xyz;
  l = normalize(q - fragment_position);
  h = normalize(l + o);

  d = distance(fragment_position, q);
  cos_nl = max(0, dot(n, l));
  cos_nh = max(0, dot(n, h));

  if (material_diffuse_texture_enabled)
    diffuse = texture(material_diffuse_texture, fragment_texcoords).rgb;
  else if (material_diffuse_color_enabled)
    diffuse = material_diffuse_color;
  else
    diffuse = vec3(0);

  falloff =
    lights[light].constant_attenuation +
    lights[light].linear_attenuation * d +
    lights[light].quadratic_attenuation * pow(d, 2);

  incident = lights[light].color.rgb * cos_nl / falloff;

  reflection = (material_specularity + 1) * pow(cos_nh, material_specularity) / (2 * PI);

  return vec3((1 - material_specularity_blending) * diffuse * incident +
	      material_specularity_blending * incident * reflection * 0.5 * cos_nl); // Blinn-Phong
  // return vec3(diffuse * incident * reflection * cos_nl); // Modèle physique
}

void main()
{
  int i;

  fragment_color.rgb = vec3(0);

  for (i = 0; i < light_count; ++i)
    fragment_color.rgb += incidentLight(i);
}

#endif
