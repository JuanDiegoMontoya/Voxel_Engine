#version 450 core

//layout (location = 0) out vec4 FragColor;
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D colorTex;
uniform sampler2D depthMap;
uniform float near_plane;
uniform float far_plane;

float LinearizeDepth(float depth)
{
  float z = depth * 2.0 - 1.0; // Back to NDC 
  return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

float map(float val, float r1s, float r1e, float r2s, float r2e)
{
  return (val - r1s) / (r1e - r1s) * (r2e - r2s) + r2s;
}

void main()
{
  float depthValue = texture(depthMap, TexCoords).r;
  float depthF = LinearizeDepth(depthValue) / far_plane;
  //FragColor = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // perspective
  
  vec3 rgb = texture(colorTex, TexCoords).rgb;
  //FragColor = vec4(mix(rgb, vec3(1), .8), 1); // normalized space or something
  //FragColor = vec4(mix(rgb.rgb, fogColor, map(clamp(depthF, 0, 1), 0, 1, fogStart, fogEnd)), 1); // normalized space or something
  FragColor = vec4(vec3(rgb), 1); // normalized space or something
}