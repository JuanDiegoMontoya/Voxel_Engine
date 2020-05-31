#version 450 core

// material properties
//in vec4 vColor;
in vec3 vPos;
in vec3 vNormal;
in vec3 vTexCoord;
in vec4 vLighting; // RGBSun

uniform vec3 viewPos;   // world space
uniform float sunAngle; // cos sun angle to normal of horizon, 0-1
uniform sampler2D textureAtlas;

uniform float fogStart; // world space
uniform float fogEnd;   // world space
uniform vec3 fogColor;

uniform sampler2DArray textures;

out vec4 fragColor;

float map(float val, float r1s, float r1e, float r2s, float r2e)
{
  return (val - r1s) / (r1e - r1s) * (r2e - r2s) + r2s;
}

// returns intensity of fog, from 0 to 1
float FogCalculation()
{
  float dist = distance(vPos, viewPos);
  return clamp(map(dist, fogStart, fogEnd, 0.0, 1.0), 0.0, 1.0);
}

void main()
{
  vec4 texColor = texture(textures, vTexCoord).rgba;
  if (texColor.a == 0) discard;
  vec3 tempColor = texColor.rgb;

  vec4 lighting = vLighting;
  lighting.a *= sunAngle;
  lighting = max(lighting, vec4(.01));
  tempColor *= max(lighting.rgb, lighting.aaa);
  // fog is applied last
  tempColor = mix(tempColor, fogColor, FogCalculation());
  fragColor = vec4(tempColor, 1.0); // alpha is always 100% or 0%
  //fragColor.rgb = texColor + fragColor.rgb * .0001;
  //fragColor = vec4(.0001 * tempColor + lighting.aaa, 1.0);
}