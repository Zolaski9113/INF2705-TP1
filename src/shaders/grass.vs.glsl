#version 330 core

layout (location = 0) in vec3 position;

void main()
{
    // Position monde transmise telle quelle : la projection se fait dans le shader de géométrie
    gl_Position = vec4(position, 1.0);
}
