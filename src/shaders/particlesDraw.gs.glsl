#version 430 core

// Un point (particule) -> un carré de 4 sommets
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in ATTRIB_VS_OUT
{
    vec4 color;
    vec2 size;
    float angle;
} attribIn[];

out ATTRIB_GS_OUT
{
    vec4 color;
    vec2 texCoord;
} attribOut;

uniform mat4 projection;

// Émet un coin du carré. corner est dans [-0.5, 0.5]² : le côté vaut donc size.
// Le décalage est appliqué en espace vue (plan xy de la caméra) : le carré fait toujours face à l'écran.
void emitCorner(vec2 corner)
{
    // Rotation de la particule sur elle-même (axe z de la vue)
    float angle = attribIn[0].angle;
    mat2 rotation = mat2(cos(angle), sin(angle), -sin(angle), cos(angle));
    vec2 offset = rotation * (corner * attribIn[0].size);

    gl_Position = projection * (gl_in[0].gl_Position + vec4(offset, 0.0, 0.0));

    // Chaque attribut de sortie est écrit pour chaque sommet
    attribOut.color = attribIn[0].color;
    attribOut.texCoord = corner + 0.5; // [-0.5, 0.5] -> [0, 1]

    EmitVertex();
}

void main()
{
    // Ordre du triangle strip : bas gauche, bas droit, haut gauche, haut droit
    emitCorner(vec2(-0.5, -0.5));
    emitCorner(vec2( 0.5, -0.5));
    emitCorner(vec2(-0.5,  0.5));
    emitCorner(vec2( 0.5,  0.5));
    EndPrimitive();
}
