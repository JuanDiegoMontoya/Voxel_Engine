#version 450 core

//layout (location = 0) out vec4 FragColor;
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D colorMap;
uniform sampler2D depthMap;
uniform float near_plane;
uniform float far_plane;

float LinearizeDepth(float depth)
{
  float z = depth * 2.0 - 1.0; // Back to NDC 
  return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{
  float depthValue = texture(depthMap, TexCoords).r;
  //FragColor = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // perspective
  
  // TODO: figure out this stupid thing
  vec3 rgb = texture(colorMap, TexCoords).rgb;
  vec4 color = vec4(rgb, 2.0) / 2 + .5; // do the thang
  vec3 real = 1-color.rgb;
  FragColor = vec4(color.rgb, 0); // normalized space or something
}