#version 450 core

// TODO: replace texture() calls with flat color OR just use a texture

struct DirLight
{
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

//uniform vec4 u_color;
in vec4 vColor;
in vec3 vNormal;

uniform DirLight dirLight;

out vec4 fragColor;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

void main()
{
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
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  
  // combine results
  vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, vTexCoords));
  vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, vTexCoords));
  vec3 specular = light.specular * spec * vec3(texture(material.specular, vTexCoords));
  return (ambient + diffuse + specular);
}