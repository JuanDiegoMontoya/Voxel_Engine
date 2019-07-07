#version 450 core

layout (location = 0) in vec3 aScreenPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

out vec3 vPos;
out vec4 vColor;
out vec3 vNormal;

void main()
{
  vPos = vec3(model * vec4(aScreenPos, 1.0));
  vColor = vec4(aColor, 1.0);
  vNormal = mat3(transpose(inverse(u_model))) * aNormal;
  
  gl_Position = u_proj * u_view * vec4(vPos, 1.0);
}
