#version 330 core

in ATTRIBS_GS_OUT
{
    vec2 texCoords;
} attribsIn;

uniform sampler2D diffuseSampler;

out vec4 FragColor;

void main()
{
    // Texture du blé : les texels presque transparents découpent la forme du brin
    vec4 texColor = texture(diffuseSampler, attribsIn.texCoords);
    if (texColor.a < 0.1)
        discard;

    // Dégradé du pied (plus sombre/vert) vers la pointe (plus clair/doré) : texCoords.y va de 0 à 1
    vec3 gradient = mix(vec3(0.55, 0.65, 0.45), vec3(1.15, 1.10, 0.95), attribsIn.texCoords.y);
    FragColor = vec4(texColor.rgb * gradient, texColor.a);
}
