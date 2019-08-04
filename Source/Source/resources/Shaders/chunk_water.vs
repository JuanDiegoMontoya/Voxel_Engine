#version 450 core

layout (location = 0) in vec3 aScreenPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in float aShininess;

const int NUM_CASCADES = 3;

uniform mat4 lightSpaceMatrix[NUM_CASCADES];
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;
uniform float u_time;

out vec3 vPos;
out vec4 vColor;
out vec3 vNormal;
out float vShininess;
out vec4 FragPosLightSpace[NUM_CASCADES];
out float ClipSpacePosZ;

float hash( float n )
{
    return fract(sin(n)*43758.5453);
}

float noise( vec3 x )
{
    // The noise function returns a value in the range -1.0f -> 1.0f

    vec3 p = floor(x);
    vec3 f = fract(x);

    f       = f*f*(3.0-2.0*f);
    float n = p.x + p.y*57.0 + 113.0*p.z;

    return mix(mix(mix( hash(n+0.0), hash(n+1.0),f.x),
                   mix( hash(n+57.0), hash(n+58.0),f.x),f.y),
               mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                   mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
}

float ripplePos(float x, float z)
{
  return sin(u_time) * (sin(x) + cos(z)) * .3;
  //return noise(vec3(x, z, u_time));
}

void main()
{
  vShininess = aShininess;
  vPos = vec3(u_model * vec4(aScreenPos, 1.0));
  float rip = ripplePos(vPos.x, vPos.z) * .5 + .5;
  if (aScreenPos.y <= 0)
    rip = 0;
  mat4 ripTran = mat4(
  1.0, 0.0, 0.0, 0.0, 
  0.0, 1.0, 0.0, 0.0, 
  0.0, 0.0, 1.0, 0.0, 
  0, -(rip), 0.0, 1.0);
  
  vColor = aColor;
  //vNormal = transpose(inverse(mat3(u_model))) * aNormal;
  for (int i = 0; i < NUM_CASCADES; i++)
    FragPosLightSpace[i] = lightSpaceMatrix[i] * vec4(vPos, 1.0);
    //FragPosLightSpace[i] = lightSpaceMatrix[i] * u_model * vec4(aScreenPos, 1.0);
  
  gl_Position = u_proj * u_view * u_model * ripTran * vec4(aScreenPos, 1.0);
  ClipSpacePosZ = gl_Position.z;
}