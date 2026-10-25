#version 330 core

#define MAX_SPOT_LIGHTS 8

in ATTRIBS_VS_OUT
{
    vec2 texCoords;
    vec3 normal;
    vec3 color;
} attribsIn;

in LIGHTS_VS_OUT
{
    vec3 obsPos;
    vec3 dirLightDir;
    
    vec3 spotLightsDir[MAX_SPOT_LIGHTS];
    vec3 spotLightsSpotDir[MAX_SPOT_LIGHTS];
} lightsIn;


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

uniform vec3 globalAmbient;

layout (std140) uniform MaterialBlock
{
    Material mat;
};

layout (std140) uniform LightingBlock
{
    DirectionalLight dirLight;
    SpotLight spotLights[MAX_SPOT_LIGHTS];
};

uniform sampler2D diffuseSampler;

out vec4 FragColor;


float computeSpot(in float openingAngle, in float exponent, in vec3 spotDir, in vec3 lightDir, in vec3 normal)
{
    float spotFactor = 0.0;
    
    float cosGamma = dot(lightDir, spotDir);
    float cosDelta = cos(radians(openingAngle));

    // Le spot n'éclaire que dans son cône et sur les faces tournées vers la lumière
    if (dot(normal, lightDir) > 0.0 && cosGamma > cosDelta)
        spotFactor = pow(cosGamma, exponent);

    return spotFactor;
}

void main()
{
   vec4 texColor = texture(diffuseSampler, attribsIn.texCoords);
    
    // On empêche les pixels transparents de s'écrire dans le stencil
    if(texColor.a < 0.1)
    {
        discard;
    }

    // Normalisation après la rasterisation
    vec3 N = normalize(attribsIn.normal);
    vec3 O = normalize(lightsIn.obsPos);

    // Émission + ambiante globale
    vec3 color = mat.emission + globalAmbient * mat.ambient * texColor.rgb;

    // Directional light

    {
        vec3 L = normalize(lightsIn.dirLightDir);
        float NdotL = max(dot(N, L), 0.0);

        vec3 ambient = dirLight.ambient * mat.ambient * texColor.rgb;
        vec3 diffuse = NdotL * dirLight.diffuse * mat.diffuse * texColor.rgb;

        vec3 specular = vec3(0.0);
        if (NdotL > 0.0)
        {
            vec3 B = normalize(L + O);
            specular = pow(max(dot(B, N), 0.0), mat.shininess) * dirLight.specular * mat.specular;
        }

        color += ambient + diffuse + specular;
    }
        
    // Spot light
    
    for(int i = 0; i < nSpotLights; i++)
    {
        float dist = length(lightsIn.spotLightsDir[i]);
        vec3 L = normalize(lightsIn.spotLightsDir[i]);
        vec3 spotDir = normalize(lightsIn.spotLightsSpotDir[i]);

        float spotFactor = computeSpot(spotLights[i].openingAngle, spotLights[i].exponent, spotDir, L, N);

        float attenuation = 1.0 - smoothstep(7.0, 10.0, dist);
        float factor = spotFactor * attenuation;

        float NdotL = max(dot(N, L), 0.0);

        vec3 ambient = spotLights[i].ambient * mat.ambient * texColor.rgb;
        vec3 diffuse = NdotL * spotLights[i].diffuse * mat.diffuse * texColor.rgb;

        vec3 specular = vec3(0.0);
        if (NdotL > 0.0)
        {
            vec3 B = normalize(L + O);
            specular = pow(max(dot(B, N), 0.0), mat.shininess) * spotLights[i].specular * mat.specular;
        }

        color += ambient + factor * (diffuse + specular);
    }
    FragColor = vec4(color, texColor.a);
}
