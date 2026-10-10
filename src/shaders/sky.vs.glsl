#version 330 core

layout (location = 0) in vec3 position;

out vec3 texCoords;

uniform mat4 mvp;

void main()
{
    texCoords = position;
    gl_Position = (mvp * vec4(position, 1.0)).xyww;
    //Comparé au tutoriel, la matrice mvp englobe déja la projection
    //On fait .xyww pour forcer la profondeur a 1.0, soit toujours le plus loin,
    //ce qui fait qu'il n'est dessiné que la ou il n'y a rien d'autre devant
}
