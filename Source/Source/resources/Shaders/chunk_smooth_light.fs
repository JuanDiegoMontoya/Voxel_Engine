#version 450 core

// TODO: replace texture() calls with flat color OR just use a texture

struct DirLight
{
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// material properties
in vec4 vColor; // since there will be no textures, this is the specular component
in vec3 vNormal;
in float vShininess;
in vec3 vPos;

uniform DirLight dirLight; // the sun
uniform vec3 viewPos;

out vec4 fragColor;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

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
  
  // combine results
  vec3 ambient  = light.ambient  * vColor.rgb; // ambient diffuse
  vec3 diffuse  = light.diffuse  * diff * vColor.rgb;// * vec3(texture(material.diffuse, vTexCoords));
  vec3 specular = light.specular * spec * vColor.rgb;// * vec3(texture(material.specular, vTexCoords));
  return (ambient + diffuse + specular);
}