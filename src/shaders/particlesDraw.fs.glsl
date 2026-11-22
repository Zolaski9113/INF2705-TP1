
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
    vec4 texel = texture(
        textureSampler,
        attribIn.texCoord
    );

    // Ignorer les fragments presque transparents
    if (texel.a * attribIn.color.a < 0.02)
        discard;

    // Teinter la texture avec la couleur
    // de la particule
    FragColor = texel * attribIn.color;
}
