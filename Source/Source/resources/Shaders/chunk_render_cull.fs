#version 460 core

layout (early_fragment_tests) in;

struct DrawArraysCommand
{
  uint count;
  uint instanceCount;
  uint first;
  uint baseInstance;
};

layout(std430, binding = 1) writeonly buffer outCmds
{
  DrawArraysCommand outDrawCommands[];
};

in flat int vID;

void main()
{
  outDrawCommands[vID].instanceCount = 1;
}
