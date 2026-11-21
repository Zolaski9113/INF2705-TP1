#version 330 core

// TODO: À remplir
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in ATTRIBS_TES_OUT
{
    // TODO: Ajouter les attributs si nécessaire
    vec3 worldPos;
} attribsIn[];

out ATTRIBS_GS_OUT
{
    // TODO: Ajouter les attributs si nécessaire
    vec2 texCoords;
    float heightRatio;
} attribsOut;


uniform mat4 mvp;

// Fonction pseudo aléatoire, utiliser le paramètre co pour avoir une valeur différente en sortie
float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    // TODO: Générer un brin d'herbe à partir du point en entrée
    vec3 baseCenter = gl_in[0].gl_Position.xyz;
    vec2 seed = attribsIn[0].worldPos.xz;
    const float PI = 3.14159265359;

    // En premier lieu, on pourra générer des rectangles avec le point comme étant le centre
    // de la base du prime.
    // Par la suite, on ajoutera un facteur aléatoire de rotation en y [0, 2pi], puis une
    // rotation en x [0, 0.1pi] pour "plier" les brins. Utiliser le facteur aléatoire.

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

    // On pourra aussi faire varié la taille avec la taille de base et la variation [0, 0.1].
    const float baseWidth = 0.5;
    const float varWidth = 0.1;
    float width = baseWidth + rand(seed + vec2(4.19, 0.73)) * varWidth;
    float halfWidth = width * 0.5;

    // Idem avec la hauteur.
    const float baseHeight = 2.4;
    const float varHeight = 0.5;
    float height = baseHeight + rand(seed + vec2(7.53, 5.11)) * varHeight;
    
    // Pour finir, émission des différents points. N'oublier pas d'écrire chaque attribut de sortie.
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

    float heights[4] = float[](0.0, 0.0, 1.0, 1.0);

    for (int i = 0; i < 4; ++i)
    {
        vec3 worldPos = baseCenter + rotation * localPositions[i];
        attribsOut.texCoords = uvs[i];
        attribsOut.heightRatio = heights[i];
        gl_Position = mvp * vec4(worldPos, 1.0);
        EmitVertex();
    }

    EndPrimitive();
}
