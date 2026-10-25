#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 texCoords;

#define MAX_SPOT_LIGHTS 8

out ATTRIBS_VS_OUT
{
    vec2 texCoords;
    vec3 normal;
    vec3 color;
} attribsOut;

out LIGHTS_VS_OUT
{
    vec3 obsPos;
    vec3 dirLightDir;
    
    vec3 spotLightsDir[MAX_SPOT_LIGHTS];
    vec3 spotLightsSpotDir[MAX_SPOT_LIGHTS];
} lightsOut;

uniform mat4 mvp;
uniform mat4 view;
uniform mat4 modelView;
uniform mat3 normalMatrix;

struct Material
{
    vec3 emission;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct DirectionalLight
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    vec3 direction;
};

struct SpotLight
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    vec3 position;
    vec3 direction;
    float exponent;
    float openingAngle;
};

uniform int nSpotLights;

layout (std140) uniform MaterialBlock
{
    Material mat;
};

layout (std140) uniform LightingBlock
{
    DirectionalLight dirLight;
    SpotLight spotLights[MAX_SPOT_LIGHTS];
};

void main()
{
    // Attribs
    gl_Position = mvp * vec4(position, 1.0);
    attribsOut.texCoords = texCoords;
    // Normale nulle (modèle sans normales) : on la fait pointer vers le haut

    vec3 n = (length(normal) < 0.0001) ? vec3(0.0, 1.0, 0.0) : normal;
    attribsOut.normal = normalMatrix * n;

    // Lights

    // Directions de lumière dans le repère de la vue
    vec3 viewPos = (modelView * vec4(position, 1.0)).xyz;

    lightsOut.obsPos = -viewPos;
    lightsOut.dirLightDir = -(mat3(view) * dirLight.direction);

    for(int i = 0; i < nSpotLights; i++)
    {
        lightsOut.spotLightsDir[i] = (view * vec4(spotLights[i].position, 1.0)).xyz - viewPos;
        lightsOut.spotLightsSpotDir[i] = -(mat3(view) * spotLights[i].direction);
    }
    
}
