#version 330 core

in ATTRIBS_GS_OUT
{
    // TODO: Ajouter les attributs si nécessaire
    vec2 texCoords;
    float heightRatio;
} attribsIn;

uniform sampler2D diffuseSampler;

out vec4 FragColor;

void main()
{
    // TODO: Utiliser la texture du blé et retirer les texels avec un alpha trop petit.
    vec4 texColor = texture(diffuseSampler, attribsIn.texCoords);
    if (texColor.a < 0.1)
        discard;

    // Dégradé de couleur du bas (plus sombre/vert) vers le haut (plus clair/doré)
    vec3 gradient = mix(vec3(0.55, 0.65, 0.45), vec3(1.15, 1.10, 0.95), attribsIn.heightRatio);
    FragColor = vec4(texColor.rgb * gradient, texColor.a);
}
