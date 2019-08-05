#version 450 core

struct DirLight
{
  vec3 direction;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};
const int NUM_CASCADES = 3;

// material properties
in vec4 vColor; // since there will be no textures, this is the diffuse component
in vec3 vNormal;
in float vShininess;
in vec3 vPos;
in vec4 FragPosLightSpace[NUM_CASCADES];
in float ClipSpacePosZ;

uniform sampler2D shadowMap[NUM_CASCADES];
uniform float cascadeEndClipSpace[NUM_CASCADES];
uniform DirLight dirLight; // the sun
uniform vec3 viewPos;
uniform vec3 lightPos;
uniform float u_time;

out vec4 fragColor;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

// WATER NOISE STUFF
float fade(float t) { return t * t * t * (t * (t * 6. - 15.) + 10.); }
vec2 smoothy(vec2 x) { return vec2(fade(x.x), fade(x.y)); }

vec2 hash(vec2 co)
{
  float m = dot(co, vec2(12.9898, 78.233));
  return fract(vec2(sin(m),cos(m))* 43758.5453) * 2. - 1.;
}

float perlinNoise(vec2 uv)
{
  vec2 PT  = floor(uv);
  vec2 pt  = fract(uv);
  vec2 mmpt= smoothy(pt);

  vec4 grads = vec4(
    dot(hash(PT + vec2(.0, 1.)), pt-vec2(.0, 1.)), dot(hash(PT + vec2(1., 1.)), pt-vec2(1., 1.)),
    dot(hash(PT + vec2(.0, .0)), pt-vec2(.0, .0)), dot(hash(PT + vec2(1., .0)), pt-vec2(1., 0.))
  );

  return 5.*mix (mix (grads.z, grads.w, mmpt.x), mix (grads.x, grads.y, mmpt.x), mmpt.y);
}

float fbm(vec2 uv)
{
  float finalNoise = 0.;
  finalNoise += .50000*perlinNoise(2.*uv);
  finalNoise += .25000*perlinNoise(4.*uv);
  finalNoise += .12500*perlinNoise(8.*uv);
  finalNoise += .06250*perlinNoise(16.*uv);
  finalNoise += .03125*perlinNoise(32.*uv);

  return finalNoise;
}

vec3 waterColorModifier()
{
  // compute ripple effect with perlin noise
  float rip = perlinNoise(vPos.xz * 3)
  + perlinNoise(vec2(vPos.xz * u_time * .005));
  vec3 rippleEffect = normalize(vec3(rip, rip, rip)) * .02;
  
  // compute view angle effect
  vec3 acuteMod = vec3(0);
  vec3 obtuseMod = vec3(0, 1, 0);
  vec3 lightDir = normalize(viewPos - vPos);
  //float angle = clamp(dot(lightDir, vNormal), 0, 1) * .2;
  float angle = abs(dot(lightDir, vNormal)) * .2;
  vec3 angleEffect = mix(acuteMod, obtuseMod, vec3(angle));
  
  return rippleEffect + angleEffect;
}

float waterAngleVisModifier()
{
  vec3 lightDir = normalize(viewPos - vPos);
  vec3 normal = normalize(vNormal);
  float angle = clamp(dot(lightDir, normal), 0, 1);
  float alpha = mix(1, 0, angle) * .08;
  return alpha;
}
// END WATER NOISE STUFF

///*
float ShadowCalculation(int cascadeIndex, vec4 fragPosLightSpace)
{
  // perform perspective divide
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  
  // transform to [0,1] range
  projCoords = projCoords * 0.5 + 0.5;
  
  // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
  float closestDepth = texture(shadowMap[cascadeIndex], projCoords.xy).r;
  
  // get depth of current fragment from light's perspective
  float currentDepth = projCoords.z;
  
  // calculate bias (based on depth map resolution and slope)
  vec3 normal = normalize(vNormal);
  vec3 lightDir = normalize(lightPos - vPos);
  float bias = max(0.0005 * (1.0 - dot(normal, lightDir)), 0.0005);
  //bias = 0.0;
  
  // check whether current frag pos is in shadow
  // PCF
  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(shadowMap[cascadeIndex], 0);
  int samples = 1;
  for(int x = -samples; x <= samples; ++x)
  {
    for(int y = -samples; y <= samples; ++y)
    {
      float pcfDepth = texture(shadowMap[cascadeIndex], projCoords.xy + vec2(x, y) * texelSize).r;
      shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
    }
  }
  shadow /= pow(samples * 2 + 1, 2);
  
  // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
  if(projCoords.z > 1.0)
    shadow = 1.0;
    
  return shadow;
}
//*/

void main()
{
  vec3 color = vColor.rgb;
  color += waterColorModifier();
  vec3 normal = normalize(vNormal);
  vec3 lightColor = dirLight.diffuse;
  
  // ambient
  vec3 ambient = dirLight.ambient * color;
  
  // diffuse
  vec3 lightDir = normalize(lightPos - vPos);
  //vec3 lightDir = -dirLight.direction;
  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuse = diff * lightColor;
  
  // specular
  vec3 viewDir = normalize(viewPos - vPos);
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = 0.0;
  vec3 halfwayDir = normalize(lightDir + viewDir);  
  spec = pow(max(dot(normal, halfwayDir), 0.0), vShininess);
  vec3 specular = spec * dirLight.specular;
  
  // calculate shadow
  float shadow = 0;
  vec3 poopoo = vec3(0);
  for (int i = 0; i < NUM_CASCADES; i++)
  {
    if (ClipSpacePosZ <= cascadeEndClipSpace[i])
    {
      poopoo[i] = .2;
      shadow = ShadowCalculation(i, FragPosLightSpace[i]);
      break;
    }
  }
  //float shadow = ShadowCalculation(FragPosLightSpace);                      
  vec3 lighting = (ambient + (1.05 - shadow * .00001) * (diffuse + specular)) * color;    
  vec4 irrelevant = vec4(lighting, 0) * 0.0001;
  
  float waterVis = waterAngleVisModifier();
  fragColor = vec4(lighting, vColor.a + waterVis);
  //fragColor = vec4(normal, 1.) + irrelevant;
}