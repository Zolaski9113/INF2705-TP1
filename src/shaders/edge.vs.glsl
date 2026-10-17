#version 330 core

layout (location = 0) in vec3 position;
layout (location = 3) in vec2 texCoords;
layout (location = 2) in vec3 normal;

uniform mat4 mvp;
out vec2 fragTexCoords;

void main()
{

    fragTexCoords = texCoords;
    gl_Position = mvp * vec4(position, 1.0);
}
