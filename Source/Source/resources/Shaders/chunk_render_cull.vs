#version 460 core

layout (location = 0) in vec3 aPos;

// instanced w/ divisor 1 (member of DIB)
layout (location = 1) in int aOffset;

layout (std430, binding = 0) readonly buffer vertexBufferData
{
  float vbo[];
};

// global info
uniform mat4 u_viewProj;
uniform uint chunk_size;

out vec3 vPos;
out flat int vID;

void main()
{
  vec3 cPos = { vbo[aOffset], vbo[aOffset+1], vbo[aOffset+2] };
  vPos = aPos + cPos * chunk_size;
  vID = gl_VertexID;
  gl_Position = u_viewProj * vec4(vPos, 1.0);
}