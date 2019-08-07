#version 450 core

#define VISUALIZE_MAPS 0

// texture buffers
#if VISUALIZE_MAPS
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
//layout (location = 3) out vec4 gDepth;
#else
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
//layout (location = 3) out vec3 gDepth;
#endif
layout (location = 2) out vec4 gAlbedoSpec;

in vec4 vColor;
in vec3 vNormal;
in vec3 vPos;
in float vShininess;


void main()
{
#if VISUALIZE_MAPS
  //gPosition = vec4(vPos, 1.0);
  gPosition = vec4(vPos, 1.0);
  gNormal = vec4(normalize(vNormal) * .5 + .5, 1.0);
  //gDepth = vec4(vec3(gl_FragCoord.z / gl_FragCoord.w / 500.), 1.0);
#else
  gPosition = vPos;
  //gPosition = gl_FragCoord.xyz;
  gNormal = normalize(vNormal);
  //gDepth = vec3(gl_FragCoord.z / gl_FragCoord.w / 500.);
#endif
  gAlbedoSpec.rgb = vColor.rgb;
  gAlbedoSpec.a = vShininess;
}