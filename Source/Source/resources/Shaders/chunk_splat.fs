#version 450 core

in flat vec3 vPos;

uniform vec3 u_viewpos;
uniform mat4 u_invProj;
uniform mat4 u_invView;
uniform vec2 u_viewportSize;

out vec4 fragColor;


bool AABBIntersect(vec3 ro, vec3 rd, vec3 minV, vec3 maxV)
{
    vec3 invR = 1.0 / rd;

    float t0, t1;

    vec3 tbot = (minV - ro) * invR;
    vec3 ttop = (maxV - ro) * invR;

    vec3 tmin = min(ttop, tbot);
    vec3 tmax = max(ttop, tbot);


    vec2 t = max(tmin.xx, tmin.yz);

    t0 = max(t.x, t.y);
    t = min(tmax.xx, tmax.yz);
    t1 = min(t.x, t.y);

    //return vec2(t0, t1); 
    if (t0 <= t1) { return true; } else { return false; }
}


// this function actually works wtf
vec3 WorldPosFromDepth(float depth)
{
  float z = depth * 2.0 - 1.0;

  vec2 TexCoord = gl_FragCoord.xy / u_viewportSize;
  vec4 clipSpacePosition = vec4(TexCoord * 2.0 - 1.0, z, 1.0);
  vec4 viewSpacePosition = u_invProj * clipSpacePosition;

  // Perspective division
  viewSpacePosition /= viewSpacePosition.w;

  vec4 worldSpacePosition = u_invView * viewSpacePosition;

  return worldSpacePosition.xyz;
}


void main()
{
  vec3 ro = WorldPosFromDepth(0);
  vec3 rd = normalize(ro - u_viewpos);
  bool intersect = AABBIntersect(ro, rd, vPos - 0.5, vPos + 0.5);

  rd = rd * .5 + .5;
  if (intersect == true)
    fragColor.xyz = ro;
  else
    discard;//fragColor.xyz = vec3(rd);
  fragColor.a = 1.0;
}