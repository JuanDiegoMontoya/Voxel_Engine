#version 450 core

layout (location = 0) in vec3 aPos;

// chunk-wide info
uniform mat4 u_viewProj;
uniform vec3 u_pos;

out flat vec3 vPos;

void main()
{
  vPos = aPos + u_pos;
  vPos += 0.5;
  gl_Position = u_viewProj * vec4(vPos, 1.0);
  gl_PointSize = 1000.0 / gl_Position.z;
}