#version 450 core

layout (location = 0) in vec3 aScreenPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in float aShininess;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;
uniform mat4 lightSpaceMatrix;

out vec3 vPos;
out vec4 vColor;
out vec3 vNormal;
out float vShininess;
out vec4 FragPosLightSpace;

void main()
{
  vShininess = aShininess;
  vPos = vec3(u_model * vec4(aScreenPos, 1.0));
  vColor = vec4(aColor, 1.0);
  vNormal = transpose(inverse(mat3(u_model))) * aNormal;
  FragPosLightSpace = lightSpaceMatrix * vec4(vPos, 1.0);
  
  gl_Position = u_proj * u_view * u_model * vec4(aScreenPos, 1.0);
}
