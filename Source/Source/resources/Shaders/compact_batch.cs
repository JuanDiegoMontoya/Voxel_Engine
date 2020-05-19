#version 460 core

#define VISIBILITY_NONE    0
#define VISIBILITY_PARTIAL 1
#define VISIBILITY_FULL    2

struct Frustum
{
  float data_[6][4];
};

struct AABB16
{
  vec4 min;
  vec4 max;
};

struct InDrawInfo
{
  uvec4 _pad01;
  uint offset;
  uint size;
  AABB16 box;
  //vec4 bMin;
  //vec4 bMax;
};

// this struct's layout cannot change
struct DrawArraysCommand
{
  uint count;
  uint instanceCount;
  uint first;
  uint baseInstance;
};

layout(std140, binding = 0) readonly buffer inData
{
  InDrawInfo inDrawData[];
};

layout(std140, binding = 1) writeonly buffer outCmds
{
  DrawArraysCommand outDrawCommands[];
};

layout(binding = 0, offset = 0) uniform atomic_uint nextIdx;

uniform vec3 u_viewpos;
uniform Frustum u_viewfrustum;
uniform uint u_vertexSize; // size of vertex in bytes
uniform float u_cullMinDist;
uniform float u_cullMaxDist;
uniform uint u_reservedVertices; // amt of reserved space (in vertices) before vertices for instanced attributes 

bool CullDistance(in AABB16 box, in vec3 pos, float minDist, float maxDist);
int CullFrustum(in AABB16 box, in Frustum frustum);

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
  int index = 0;
  int stride = 1;

  for (int i = index; i < inDrawData.length(); i += stride)
  //for (int i = index; i < 1; i += stride)
  {
    InDrawInfo alloc = inDrawData[i];
    bool condition = // all conditions must be true to draw chunk
      CullDistance(alloc.box, u_viewpos, u_cullMinDist, u_cullMaxDist) &&
      CullFrustum(alloc.box, u_viewfrustum) >= VISIBILITY_PARTIAL;
    if (condition == true)
    {
      DrawArraysCommand cmd;
      cmd.count = (alloc.size / u_vertexSize) - u_reservedVertices;
      cmd.instanceCount = 1;
      cmd.first = alloc.offset / u_vertexSize;
      cmd.baseInstance = cmd.first;

      uint insert = atomicCounterAdd(nextIdx, 1);
      outDrawCommands[insert] = cmd;
    }
  }
}


bool CullDistance(in AABB16 box, in vec3 pos, float minDist, float maxDist)
{
  vec3 bp = (box.max.xyz + box.min.xyz) / 2.0;
  float dist = distance(bp, pos);
  return dist >= minDist && dist <= maxDist;
}


vec4 GetPlane(int plane, in Frustum frustum)
{
  return vec4(frustum.data_[plane][0], frustum.data_[plane][1], 
    frustum.data_[plane][2], frustum.data_[plane][3]);
}


int GetVisibility(in vec4 clip, in AABB16 box)
{
  float x0 = box.min.x * clip.x;
  float x1 = box.max.x * clip.x;
  float y0 = box.min.y * clip.y;
  float y1 = box.max.y * clip.y;
  float z0 = box.min.z * clip.z + clip.w;
  float z1 = box.max.z * clip.z + clip.w;
  float p1 = x0 + y0 + z0;
  float p2 = x1 + y0 + z0;
  float p3 = x1 + y1 + z0;
  float p4 = x0 + y1 + z0;
  float p5 = x0 + y0 + z1;
  float p6 = x1 + y0 + z1;
  float p7 = x1 + y1 + z1;
  float p8 = x0 + y1 + z1;

  if (p1 <= 0 && p2 <= 0 && p3 <= 0 && p4 <= 0 && p5 <= 0 && p6 <= 0 && p7 <= 0 && p8 <= 0)
  {
    return VISIBILITY_NONE;
  }
  if (p1 > 0 && p2 > 0 && p3 > 0 && p4 > 0 && p5 > 0 && p6 > 0 && p7 > 0 && p8 > 0)
  {
    return VISIBILITY_FULL;
  }

  return VISIBILITY_PARTIAL;
}


int CullFrustum(in AABB16 box, in Frustum frustum)
{
  int v0 = GetVisibility(GetPlane(0, frustum), box);
  if (v0 == VISIBILITY_NONE)
  {
    return VISIBILITY_NONE;
  }

  int v1 = GetVisibility(GetPlane(1, frustum), box);
  if (v1 == VISIBILITY_NONE)
  {
    return VISIBILITY_NONE;
  }

  int v2 = GetVisibility(GetPlane(2, frustum), box);
  if (v2 == VISIBILITY_NONE)
  {
    return VISIBILITY_NONE;
  }

  int v3 = GetVisibility(GetPlane(3, frustum), box);
  if (v3 == VISIBILITY_NONE)
  {
    return VISIBILITY_NONE;
  }

  int v4 = GetVisibility(GetPlane(4, frustum), box);
  if (v4 == VISIBILITY_NONE)
  {
    return VISIBILITY_NONE;
  }

  if (v0 == VISIBILITY_FULL && v1 == VISIBILITY_FULL &&
    v2 == VISIBILITY_FULL && v3 == VISIBILITY_FULL &&
    v4 == VISIBILITY_FULL)
  {
    return VISIBILITY_FULL;
  }

  return VISIBILITY_PARTIAL;
}