#version 430 core

in ATTRIB_GS_OUT
{
    vec4 color;
    vec2 texCoord;
} attribIn;

out vec4 FragColor;

uniform sampler2D textureSampler;

void main()
{
    vec4 texel = texture(textureSampler, attribIn.texCoord);

    // Texels invisibles : on ne les traite pas
    if (texel.a < 0.02)
        discard;

    // Texture teintée par la couleur de la particule (l'alpha porte aussi le fondu)
    FragColor = texel * attribIn.color;
}
