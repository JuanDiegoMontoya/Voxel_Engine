#version 460 core

struct Frustum
{
  //...
};

struct InDrawInfo
{
  //...

};

struct DrawArraysCommand
{
  uint count;
  uint instanceCount;
  uint first;
  uint baseInstance;
};

layout(std430, binding = 0) buffer inData
{
  InDrawInfo inDrawInfo[];
}

layout(std430, binding = 1) buffer outCmds
{
  DrawArraysCommand outDrawCommands[];
}

layout(binding = 0, offset = 0) uniform atomic_uint nextIdx;

layout(local_size_variable) in;
void main()
{
  int index;
  int stride;

  for (int i = index; i < inDrawInfo.length(); i += stride)
  {
    if (condition)
    {
      DrawArraysCommand cmd;
      cmd.count = ...;
      cmd.instanceCount = 1;
      cmd.first = ...;
      cmd.baseInstance = ...;

      int insert = atomicCounterAdd(nextIdx, 1);
      outDrawCommands[insert] = cmd;
    }
  }
}