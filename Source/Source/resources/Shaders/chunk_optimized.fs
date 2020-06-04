#version 450 core

// material properties
//in vec4 vColor;
in vec3 vPos;
in vec3 vNormal;
in vec3 vTexCoord;
in vec4 vLighting; // RGBSun

uniform vec3 viewPos;   // world space
uniform float sunAngle; // cos sun angle to normal of horizon, 0-1
uniform sampler2D textureAtlas;

uniform float fogStart; // world space
uniform float fogEnd;   // world space
uniform vec3 fogColor;

uniform sampler2DArray textures;
uniform sampler2D blueNoise;

out vec4 fragColor;

float map(float val, float r1s, float r1e, float r2s, float r2e)
{
  return (val - r1s) / (r1e - r1s) * (r2e - r2s) + r2s;
}

// returns intensity of fog, from 0 to 1
float FogCalculation()
{
  float dist = distance(vPos, viewPos);
  return clamp(map(dist, fogStart, fogEnd, 0.0, 1.0), 0.0, 1.0);
}

float mod289(float x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 mod289(vec4 x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 perm(vec4 x){return mod289(((x * 34.0) + 1.0) * x);}

float noise(vec3 p){
    vec3 a = floor(p);
    vec3 d = p - a;
    d = d * d * (3.0 - 2.0 * d);

    vec4 b = a.xxyy + vec4(0.0, 1.0, 0.0, 1.0);
    vec4 k1 = perm(b.xyxy);
    vec4 k2 = perm(k1.xyxy + b.zzww);

    vec4 c = k2 + a.zzzz;
    vec4 k3 = perm(c);
    vec4 k4 = perm(c + 1.0);

    vec4 o1 = fract(k3 * (1.0 / 41.0));
    vec4 o2 = fract(k4 * (1.0 / 41.0));

    vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);
    vec2 o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);

    return o4.y * d.y + o4.x * (1.0 - d.y);
}

void main()
{
  vec4 texColor = texture(textures, vTexCoord).rgba;
  //if (texColor.a == 0) discard;
  vec3 tempColor = texColor.rgb;

  vec4 lighting = vLighting;
  lighting.a *= sunAngle;
  lighting = max(lighting, vec4(.01));
  tempColor *= max(lighting.rgb, lighting.aaa);
  // fog is applied last
  tempColor = mix(tempColor, fogColor, FogCalculation());
  fragColor = vec4(tempColor, 1.0); // alpha is always 100% or 0%
  //fragColor.rgb = texColor + fragColor.rgb * .0001;
  //fragColor = vec4(.0001 * tempColor + lighting.aaa, 1.0);

  // Fancy dithered alpha stuff
  float opacity = texColor.a;

#if 0 // blue noise dithering (if false)
#if 1 // uniform sampling
  int x = int(gl_FragCoord.x) % 4;
  int y = int(gl_FragCoord.y) % 4;
#else
  int x = (int(gl_FragCoord.x) + int(3.7 * noise(ivec3(vPos)))) % 4;
  int y = (int(gl_FragCoord.y) + int(2.1 * noise(ivec3(vPos)))) % 4;
#endif // uniform sampling
  int index = x + y * 4;
  float limit = 0.0;

  const mat4 thresholdMatrix = mat4
  (
    1.0 / 16.0,  9.0 / 16.0,  3.0 / 16.0, 11.0 / 16.0,
    13.0 / 16.0,  5.0 / 16.0, 15.0 / 16.0,  7.0 / 16.0,
    4.0 / 16.0, 12.0 / 16.0,  2.0 / 16.0, 10.0 / 16.0,
    16.0 / 16.0,  8.0 / 16.0, 14.0 / 16.0,  6.0 / 16.0
  );
  limit = thresholdMatrix[x][y];
#else
  ivec2 md = textureSize(blueNoise, 0);
  vec2 coord = gl_FragCoord.xy + gl_FragCoord.z * 50;
  ivec2 uv = ivec2(int(coord.x) % md.x, int(coord.y) % md.y);
  float limit = texture(blueNoise, vec2(uv) / vec2(md)).r;
#endif // blue noise dithering

  // Is this pixel below the opacity limit? Skip drawing it
  if (opacity < limit)
    discard;
}