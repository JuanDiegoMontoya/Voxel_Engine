#version 450 core

// aEncoded layout (left to right bits):
// 0 - 17   18 - 20   21 - 31
// vertex   normal    texcoord
// vertex = x, y, z from 0-32 (supports up to 63)
// normal = 0 - 5 index into "normals" table
// texcoord = texture index (0 - 512), corner index (0 - 3)
layout (location = 0) in float aEncoded; // encoded uint as float (per vertex)

// aLighting layout
// 0 - 15   16 - 19   20 - 23   24 - 27   28 - 31
// unused     R         G         B         Sun
layout (location = 1) in float aLighting;// encoded uint as float (per vertex)

layout (location = 2) in vec3 aChunkPos; // (per instance) 

// global info
uniform mat4 u_viewProj;

// per-chunk info
//uniform vec3 u_pos;

out vec3 vPos;
out vec3 vNormal;
out vec2 vTexCoord;
out vec4 vLighting; // RGBSun

out vec4 vColor;

const vec3 normals[] =
{
  { 0, 0, 1 }, // 'far' face    (+z direction)
  { 0, 0,-1 }, // 'near' face   (-z direction)
  {-1, 0, 0 }, // 'left' face   (-x direction)
  { 1, 0, 0 }, // 'right' face  (+x direction)
  { 0, 1, 0 }, // 'top' face    (+y direction)
  { 0,-1, 0 }  // 'bottom' face (-y direction)
};

// clockwise from bottom left texture coordinates
const vec2 tex_corners[] =
{
  { 0, 0 },
  { 0, 1 },
  { 1, 1 },
  { 1, 0 }
};

// decodes vertex, normal, and texcoord info from encoded data
// returns usable data (i.e. fully processed)
void Decode(in uint encoded,
  out vec3 modelPos, out vec3 normal, out vec2 texCoord)
{
  // decode vertex position
  modelPos.x = encoded >> 26;
  modelPos.y = (encoded >> 20) & 0x3F; // = 0b111111
  modelPos.z = (encoded >> 14) & 0x3F; // = 0b111111
  //modelPos += 0.5;

  // decode normal
  uint normalIdx = (encoded >> 11) & 0x7; // = 0b111
  normal = normals[normalIdx];

  // decode texture index and UV
  uint textureIdx = (encoded >> 2) & 0x1FF; // = 0b1111111111
  uint cornerIdx = (encoded >> 0) & 0x3; // = 0b11
  vec2 corner = tex_corners[cornerIdx];

  // sample from texture using knowledge of texture dimensions and block index
  // texCoord = ...
}


// decodes lighting information into a usable vec4
void Decode(in uint encoded, out vec4 lighting)
{
  lighting.r = encoded >> 12;
  lighting.g = (encoded >> 8) & 0xF;
  lighting.b = (encoded >> 4) & 0xF;
  lighting.a = encoded & 0xF;
  lighting = (0.0 + lighting) / 16.0;
}


void main()
{
  vec3 modelPos;
  Decode(floatBitsToUint(aEncoded), modelPos, vNormal, vTexCoord);
  vPos = modelPos + aChunkPos;
  Decode(floatBitsToUint(aLighting), vLighting);
  //vColor = aColor;

  gl_Position = u_viewProj * vec4(vPos, 1.0);
}