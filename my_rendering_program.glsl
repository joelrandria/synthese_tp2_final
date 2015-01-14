#version 330

#ifdef VERTEX_SHADER

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 texcoords;

uniform mat4 m_matrix;
uniform mat4 m_normalmatrix;
uniform mat4 mv_matrix;
uniform mat4 mvp_matrix;
uniform mat4 mv_normalmatrix;

out vec3 fragment_world_position;
out vec3 fragment_view_position;
out vec3 fragment_world_normal;
out vec3 fragment_view_normal;
out vec2 fragment_texcoords;

void main()
{
  gl_Position = mvp_matrix * vec4(position, 1);

  fragment_world_position = (m_matrix * vec4(position, 1)).xyz;
  fragment_view_position = (mv_matrix * vec4(position, 1)).xyz;
  fragment_world_normal = (m_normalmatrix * vec4(normal, 0)).xyz;
  fragment_view_normal = (mv_normalmatrix * vec4(normal, 0)).xyz;
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

  mat4 shadowmap_vp_matrix;
  mat4 shadowmap_vpi_matrix;

  // Champs inutilisés
  uint shadow_framebuffer;
  uint shadow_depth_texture;
};

in vec3 fragment_world_position;
in vec3 fragment_view_position;
in vec3 fragment_world_normal;
in vec3 fragment_view_normal;
in vec2 fragment_texcoords;

uniform mat4 v_matrix;

uniform int light_count;
uniform point_light_buffer
{
  point_light lights[POINT_LIGHTS_MAX_COUNT];
};
uniform sampler2D light_shadow_depth_texture0;
uniform sampler2D light_shadow_depth_texture1;
uniform sampler2D light_shadow_depth_texture2;
uniform sampler2D light_shadow_depth_texture3;
uniform sampler2D light_shadow_depth_texture4;

uniform bool material_diffuse_color_enabled;
uniform bool material_diffuse_texture_enabled;

uniform vec3 material_diffuse_color;
uniform sampler2D material_diffuse_texture;

uniform float material_specularity;
uniform float material_specularity_blending;

out vec4 fragment_color;

bool isVisible(int light)
{
  float map_depth;

  mat4 light_vpi_mat = transpose(lights[light].shadowmap_vpi_matrix); // conversion "row-major"

  vec4 ph = light_vpi_mat * vec4(fragment_world_position, 1);
  vec2 uv = (ph / ph.w).xy;

  switch (light)
  {
  case 0: map_depth = texture(light_shadow_depth_texture0, uv).r; break;
  case 1: map_depth = texture(light_shadow_depth_texture1, uv).r; break;
  case 2: map_depth = texture(light_shadow_depth_texture2, uv).r; break;
  case 3: map_depth = texture(light_shadow_depth_texture3, uv).r; break;
  case 4: map_depth = texture(light_shadow_depth_texture4, uv).r; break;
  default: return false;
  }

  vec4 p = vec4(fragment_world_position +  normalize(fragment_world_normal), 1);
  vec4 ph2 = light_vpi_mat * p;

  return ((ph2 / ph2.w).z <= map_depth);
}

vec3 reflectedLight(int light, vec3 n, vec3 o, vec3 diffuse)
{
  vec3 q;
  vec3 l;
  vec3 h;

  float d;
  float cos_nl;
  float cos_nh;
  float falloff;

  vec3 incident;
  float reflection;

  q = (v_matrix * vec4(lights[light].position.xyz, 1)).xyz;
  l = normalize(q - fragment_view_position);
  h = normalize(l + o);

  d = distance(fragment_view_position, q);
  cos_nl = max(0, dot(n, l));
  cos_nh = max(0, dot(n, h));

  falloff =
    lights[light].constant_attenuation +
    lights[light].linear_attenuation * d +
    lights[light].quadratic_attenuation * pow(d, 2);

  incident = lights[light].color.rgb * cos_nl / falloff;

  reflection = (material_specularity + 1) * pow(cos_nh, material_specularity) / (2 * PI);

  // Blinn-Phong
  return vec3((1 - material_specularity_blending) * diffuse * incident +
  	      material_specularity_blending * incident * reflection * 0.5 * cos_nl);

  // Modèle physique
  // return vec3(diffuse * incident * reflection * cos_nl);
}

void main()
{
  int i;

  vec3 n;
  vec3 o;

  vec3 diffuse;

  n = normalize(fragment_view_normal);
  o = normalize(fragment_view_position * -1);

  if (material_diffuse_texture_enabled)
    diffuse = texture(material_diffuse_texture, fragment_texcoords).rgb;
  else if (material_diffuse_color_enabled)
    diffuse = material_diffuse_color;
  else
    diffuse = vec3(0);

  fragment_color.rgb = vec3(0);
  for (i = 0; i < light_count; ++i)
    if (isVisible(i))
      fragment_color.rgb += reflectedLight(i, n, o, diffuse);
}

#endif
