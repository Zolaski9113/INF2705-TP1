#version 330 core

in vec2 fragTexCoords;
out vec4 FragColor;
uniform sampler2D fenceTexture;

void main()
{
    vec4 texColor = texture(fenceTexture, fragTexCoords);

    // On efface uniquement les pixels transparents
    if(texColor.a < 0.1)
    {
       discard;
    }

    // Couleur solide noire pour l'effet de contour
    FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    
}
