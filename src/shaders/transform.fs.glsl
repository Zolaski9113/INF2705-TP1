#version 330 core

// TODO: La couleur des fragments est donnée à partir de la couleur
//       des vertices passée en entrée.

in vec3 frag_color;

out vec4 out_color;

void main()
{
    out_color = vec4(frag_color, 1.0);
}
