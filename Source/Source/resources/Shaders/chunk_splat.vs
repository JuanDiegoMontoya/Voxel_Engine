#version 450 core

layout (location = 0) in float aEncoded;

// chunk-wide info
uniform mat4 u_viewProj;
uniform vec3 u_pos;
uniform vec2 u_viewportSize;

out flat vec3 vPos;
out vec3 vColor;


void Decode(in uint encoded, out vec3 modelPos, out vec3 color)
{
  // decode vertex position
  modelPos.x = encoded >> 26;
  modelPos.y = (encoded >> 20) & 0x3F; // = 0b111111
  modelPos.z = (encoded >> 14) & 0x3F; // = 0b111111

  color.r = (encoded >> 10) & 0xF;
  color.g = (encoded >> 6) & 0xF;
  color.b = (encoded >> 2) & 0xF;
  color /= (1 << 4);
}

void SetPointSizeFancy(vec4 glPos)
{
  float aspectRatio = u_viewportSize.x / u_viewportSize.y;
  // embiggen point if it's near screen edge
  float reduce = max(
    abs(glPos.x * aspectRatio / glPos.w),
    abs(glPos.y / glPos.w)
  );
  reduce -= 0.13; // affects overdraw near edges
  float size = (u_viewportSize.y * 1.1) / glPos.z * max(reduce, 1.0);
  gl_PointSize = size * 1.0;
}

void main()
{
  vec3 modelPos;
  Decode(floatBitsToUint(aEncoded), modelPos, vColor);
  vPos = modelPos + u_pos;
  vPos += 0.5;
  gl_Position = u_viewProj * vec4(vPos, 1.0);

  // extremely unscientific pointsize calculation
  // let's just say it was done in the name of performance
  gl_PointSize = 1200.0 / gl_Position.z;
  SetPointSizeFancy(gl_Position);
}