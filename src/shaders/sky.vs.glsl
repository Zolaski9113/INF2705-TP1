#version 330 core

layout (location = 0) in vec3 position;

out vec3 texCoords;

uniform mat4 mvp;

void main()
{
    texCoords = position;
    gl_Position = (mvp * vec4(position, 1.0)).xyww;
    // mvp = projection × vue sans translation (calculée en C++) : le ciel suit la caméra sans s'en approcher.
    // .xyww force la profondeur à 1.0 : avec GL_LEQUAL, le ciel n'apparaît que là où rien n'est dessiné.
}
