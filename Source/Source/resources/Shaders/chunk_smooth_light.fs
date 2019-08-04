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

out vec4 fragColor;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

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
  float bias = max(0.001 * (1.0 - dot(normal, lightDir)), 0.001);
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
    shadow = 0.0;
    
  return shadow;
}
//*/
/*
float ShadowCalculation(int cascadeIndex, vec4 fragPosLightSpace)
{
  vec3 ProjCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

  vec2 UVCoords;
  UVCoords.x = 0.5 * ProjCoords.x + 0.5;
  UVCoords.y = 0.5 * ProjCoords.y + 0.5;

  float z = 0.5 * ProjCoords.z + 0.5;
  float Depth = texture(shadowMap[cascadeIndex], UVCoords).x;

  if (Depth < z + 0.00001)
    return 0.0;
  else
    return 1.0;
}
*/

void main()
{
  vec3 color = vColor.rgb;
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
    if (ClipSpacePosZ <= (cascadeEndClipSpace[i]))
    {
      poopoo[i] = .2;
      shadow = ShadowCalculation(i, FragPosLightSpace[i]);
      break;
    }
  }
  //float shadow = ShadowCalculation(FragPosLightSpace);                      
  vec3 lighting = (ambient + (1.0 - shadow * .000001) * (diffuse + specular)) * color;    
  vec4 irrelevant = vec4(lighting, 0) * 0.0001;
  
  //fragColor = vec4(poopoo, 1) + irrelevant;
  //fragColor = vec4(lighting, vColor.a);
  //fragColor = vec4(vec3(FragPosLightSpace[0].y / 10), 1) + irrelevant;
  //fragColor = vec4(vec3(ClipSpacePosZ) / 100, 1) + irrelevant;
  fragColor = vec4(vec3(shadow / 3) + vec3(ClipSpacePosZ / 200) + poopoo, 1) + irrelevant;
}

/*
void main()
{
  // properties
  vec3 norm = normalize(vNormal);
  vec3 viewDir = normalize(viewPos - vPos);
  
  vec3 result = CalcDirLight(dirLight, norm, viewDir);
  fragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
  vec3 lightDir = normalize(-light.direction);
  
  // diffuse shading
  float diff = max(dot(normal, lightDir), 0.0);
  
  // specular shading
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), vShininess);
  //vec3 halfwayDir = normalize(lightDir + viewDir);
  //spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
  
  // combine results
  vec3 ambient  = light.ambient  * vColor.rgb; // ambient diffuse
  vec3 diffuse  = light.diffuse  * diff * vColor.rgb;// * vec3(texture(material.diffuse, vTexCoords));
  vec3 specular = light.specular * spec * vColor.rgb;// * vec3(texture(material.specular, vTexCoords));
  float shadow = ShadowCalculation(FragPosLightSpace);
  //return ((ambient + (1.0 - shadow)) * (diffuse + specular) * vColor.rgb);
  //return ambient + ((1.0 - shadow) * (diffuse + specular));
  return vec3(1.0 - shadow) + .001 * (specular * diffuse * ambient * shadow);
}
*/