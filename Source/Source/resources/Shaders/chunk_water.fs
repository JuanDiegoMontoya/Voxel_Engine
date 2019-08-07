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

// ssr
//in vec2 ssTexCoords;
in vec3 camNormal; // view space
vec2 ssTexCoords;
uniform sampler2D ssr_positions;
uniform sampler2D ssr_normals;
uniform sampler2D ssr_albedoSpec;
uniform sampler2D ssr_depth;
uniform mat4 u_view;
uniform mat4 u_proj;
uniform vec3 ssr_skyColor;
uniform mat4 inv_projection;

out vec4 fragColor;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);





vec3 calcViewPosition(in vec2 texCoord)
{
  // Combine UV & depth into XY & Z (NDC)
  //vec3 rawPosition = vec3(texCoord, texture(ssr_depth, texCoord).r);
  vec3 rawPosition = vec3(texCoord, gl_FragCoord.z);
  
  // Convert from (0, 1) range to (-1, 1)
  vec4 ScreenSpacePosition = vec4(rawPosition * 2. - 1., 1.);
  
  // Undo Perspective transformation to bring into view space
  vec4 ViewPosition = inv_projection * ScreenSpacePosition;
  
  // Perform perspective divide and return
  return ViewPosition.xyz / ViewPosition.w;
}

vec3 calcViewPositionDepthTex(in vec2 texCoord)
{
  vec3 rawPosition = vec3(texCoord, texture(ssr_depth, texCoord).r);
  vec4 ScreenSpacePosition = vec4(rawPosition * 2. - 1., 1.);
  vec4 ViewPosition = inv_projection * ScreenSpacePosition;
  return ViewPosition.xyz / ViewPosition.w;
}

vec2 binarySearch(inout vec3 dir, inout vec3 hitCoord, inout float dDepth)
{
  float depth;

  vec4 projectedCoord;

  for(int i = 0; i < 20; i++) {
    projectedCoord = u_proj * vec4(hitCoord, 1.0);
    projectedCoord.xy /= projectedCoord.w;
    projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

    //depth = texture(ssr_positions, projectedCoord.xy).z;
    depth = calcViewPositionDepthTex(projectedCoord.xy).z;

    dDepth = hitCoord.z - depth;

    dir *= 0.5;
    if(dDepth > 0.0)
      hitCoord += dir;
    else
      hitCoord -= dir;    
  }

  projectedCoord = u_proj * vec4(hitCoord, 1.0);
  projectedCoord.xy /= projectedCoord.w;
  projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

  return vec2(projectedCoord.xy);
}

vec2 rayCast(vec3 dir, inout vec3 hitCoord, out float dDepth)
{
  dir *= .3;

  for (int i = 0; i < 20; i++)
  {
    hitCoord += dir; 
    
    // convert 3D coords into 2D screenspace
    vec4 projectedCoord = u_proj * vec4(hitCoord, 1.0);
    projectedCoord.xy /= projectedCoord.w;
    projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5; 

    float depth = calcViewPositionDepthTex(projectedCoord.xy).z;
    dDepth = hitCoord.z - depth;

    if((dir.z - dDepth) < 1.2 && dDepth < 0.0 && i > 0)
      //return projectedCoord.xy;
      return binarySearch(dir, hitCoord, dDepth);
  }

  return vec2(-1.0f);
}

vec3 ssr()
{
  ssTexCoords.x = gl_FragCoord.x / textureSize(ssr_positions, 0).x;
  ssTexCoords.y = gl_FragCoord.y / textureSize(ssr_positions, 0).y;
  vec2 texCoord = ssTexCoords;
  
  //vec3 normal = texture(ssr_normals, texCoord).xyz * 2.0 - 1.0;
  //vec3 normal = normalize(vNormal);
  // normal at initial ray position
  vec3 normal = camNormal;
  vec3 fViewPos = calcViewPosition(texCoord);
  
  // Reflection vector
  vec3 reflected = normalize(reflect(normalize(fViewPos), normalize(normal)));
  
  // Ray cast
  vec3 hitPos = fViewPos;
  float dDepth;
  float minRayStep = .05f;
  vec2 coords = rayCast(reflected * max(minRayStep, -fViewPos.z), hitPos, dDepth);
  if (coords != vec2(-1.0))
    return mix(vec3(0), texture(ssr_albedoSpec, coords).rgb / 2, .5);
  else
    return vec3(0);
}




/*
// Create a float value 0 to 1 into a color from red, through green and then blue.
vec4 rainbow(float x) {
  float level = x * 2.0;
  float r, g, b;
  if (level <= 0) {
    r = g = b = 0;
  } else if (level <= 1) {
    r = mix(1, 0, level);
    g = mix(0, 1, level);
    b = 0;
  } else if (level > 1) {
    r = 0;
    g = mix(1, 0, level-1);
    b = mix(0, 1, level-1);
  }
  return vec4(r, g, b, 1);
}
 
//uniform sampler2D colTex;     // Color texture sampler
//uniform sampler2D posTex;     // World position texture sampler
//uniform sampler2D normalTex;  // Normal texture sampler
//in vec2 screen;               // The screen position (0 to 1)
//layout(location = 0) out vec4 color;
 
vec3 ssr()
{
  vec4 color = vec4(1, 0, 0, 1);
  
  //vec3 worldStartingPos = texture(posTex, screen).xyz;
  vec3 worldStartingPos = vPos.xyz;
  //vec3 normal = texture(normalTex, screen).xyz;
  vec3 normal = camNormal.xyz;
  //vec3 cameraToWorld = worldStartingPos.xyz - UBOCamera.xyz;
  vec3 cameraToWorld = worldStartingPos.xyz - viewPos.xyz;
  float cameraToWorldDist = length(cameraToWorld);
  vec3 cameraToWorldNorm = normalize(cameraToWorld);
  vec3 refl = normalize(reflect(cameraToWorldNorm, normal)); // This is the reflection vector

  if (dot(refl, cameraToWorldNorm) < 0) {
    // Ignore reflections going backwards towards the camera, indicate with blue
    color = vec4(0,0,1,1);
    return color.xyz;
  }

  vec3 newPos;
  vec4 newScreen;
  float i = 0;
  vec3 rayTrace = worldStartingPos;
  float currentWorldDist, rayDist;
  float incr = 0.4;
  do {
    i += 0.05;
    rayTrace += refl*incr;
    incr *= 1.3;
    //newScreen = UBOProjectionviewMatrix * vec4(rayTrace, 1);
    newScreen = u_proj * u_view * vec4(rayTrace, 1);
    newScreen /= newScreen.w;
    //newPos = texture(posTex, newScreen.xy/2.0+0.5).xyz;
    newPos = texture(ssr_positions, newScreen.xy/2.0+0.5).xyz;
    //currentWorldDist = length(newPos.xyz - UBOCamera.xyz);
    currentWorldDist = length(newPos.xyz - viewPos.xyz);
    //rayDist = length(rayTrace.xyz - UBOCamera.xyz);
    rayDist = length(rayTrace.xyz - viewPos.xyz);
    if (newScreen.x > 1 || newScreen.x < -1 || newScreen.y > 1 || newScreen.y < -1 || newScreen.z > 1 || newScreen.z < -1 || i >= 1.0 || cameraToWorldDist > currentWorldDist) {
      break; // This is a failure mode.
    }
  } while(rayDist < currentWorldDist);

  if (cameraToWorldDist > currentWorldDist)
    color = vec4(1,1,0,1); // Yellow indicates we found a pixel hidden behind another object
  else if (newScreen.x > 1 || newScreen.x < -1 || newScreen.y > 1 || newScreen.y < -1)
    color = vec4(0,0,0,1); // Black used for outside of screen
  else if (newScreen.z > 1 && newScreen.z < -1)
    color = vec4(1,1,1,1); // White outside of frustum
  else
    color = rainbow(i); // Encode number of iterations as a color. Red, then green and last blue
  return color.xyz + texture(ssr_albedoSpec, vec2(0)).xyz * .0001 + texture(ssr_depth, vec2(0)).xyz * .0001;
}
*/





// WATER NOISE STUFF
float fade(float t) { return t * t * t * (t * (t * 6. - 15.) + 10.); }
vec2 smoothy(vec2 x) { return vec2(fade(x.x), fade(x.y)); }

vec2 hash2(vec2 co)
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
    dot(hash2(PT + vec2(.0, 1.)), pt-vec2(.0, 1.)), dot(hash2(PT + vec2(1., 1.)), pt-vec2(1., 1.)),
    dot(hash2(PT + vec2(.0, .0)), pt-vec2(.0, .0)), dot(hash2(PT + vec2(1., .0)), pt-vec2(1., 0.))
  );

  return 5.*mix (mix (grads.z, grads.w, mmpt.x), mix (grads.x, grads.y, mmpt.x), mmpt.y);
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
  fragColor = vec4(lighting + ssr() * .3, vColor.a + waterVis);
  //fragColor = vec4(ssr(), 0.0) * .5 + fragColor;
  //fragColor = vec4(vNormal * .5 + .5, 1.0) + fragColor * ssr().rrrr * .00001;
  //fragColor = vec4(ssTexCoords, 0.0, 1.0) + (fragColor + vec4(ssr(), 0)) * .00001;
  //fragColor = vec4(normal, 1.) + irrelevant;
}