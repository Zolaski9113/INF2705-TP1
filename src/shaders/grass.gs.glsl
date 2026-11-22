#version 330 core

// Un point (base du brin) -> un rectangle de 4 sommets
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

out ATTRIBS_GS_OUT
{
    vec2 texCoords;
} attribsOut;


uniform mat4 mvp;

// Fonction pseudo aléatoire, utiliser le paramètre co pour avoir une valeur différente en sortie
float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    // Position monde de la base ; sa position xz sert de graine : un même brin garde sa forme d'une trame à l'autre
    vec3 baseCenter = gl_in[0].gl_Position.xyz;
    vec2 seed = baseCenter.xz;
    const float PI = 3.14159265359;

    // Rotation aléatoire en y [0, 2pi] (orientation), puis en x [0, 0.1pi] pour "plier" le brin.
    // Décalages différents de la graine => valeurs aléatoires indépendantes.
    float angleY = rand(seed) * 2.0 * PI;
    float angleX = rand(seed + vec2(1.37, 2.91)) * 0.1 * PI;

    mat3 rotY = mat3(
        cos(angleY),  0.0, sin(angleY),
        0.0,          1.0, 0.0,
        -sin(angleY), 0.0, cos(angleY)
    );

    mat3 rotX = mat3(
        1.0, 0.0,         0.0,
        0.0, cos(angleX), -sin(angleX),
        0.0, sin(angleX), cos(angleX)
    );

    mat3 rotation = rotY * rotX;

    // Taille variable : base + variation aléatoire, en largeur et en hauteur.
    const float baseWidth = 0.5;
    const float varWidth = 0.1;
    float width = baseWidth + rand(seed + vec2(4.19, 0.73)) * varWidth;
    float halfWidth = width * 0.5;

    const float baseHeight = 2.4;
    const float varHeight = 0.5;
    float height = baseHeight + rand(seed + vec2(7.53, 5.11)) * varHeight;

    // Rectangle dans le repère local du brin (base centrée sur le point), émis en triangle strip.
    // uv.y va de 0 (pied) à 1 (pointe) : sert aussi au dégradé dans le fragment shader.
    vec3 localPositions[4] = vec3[](
        vec3(-halfWidth, 0.0,    0.0), // Bas gauche
        vec3( halfWidth, 0.0,    0.0), // Bas droit
        vec3(-halfWidth, height, 0.0), // Haut gauche
        vec3( halfWidth, height, 0.0)  // Haut droit
    );

    vec2 uvs[4] = vec2[](
        vec2(0.0, 0.0),
        vec2(1.0, 0.0),
        vec2(0.0, 1.0),
        vec2(1.0, 1.0)
    );

    for (int i = 0; i < 4; ++i)
    {
        vec3 worldPos = baseCenter + rotation * localPositions[i];
        attribsOut.texCoords = uvs[i];
        gl_Position = mvp * vec4(worldPos, 1.0);
        EmitVertex();
    }

    EndPrimitive();
}
