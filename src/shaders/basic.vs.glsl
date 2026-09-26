#version 330 core

layout(location = 0) in vec2 aPos;     // position (x, y)
layout(location = 1) in vec3 aRgb;    // couleur (r, g, b)
out vec4 laCouleur;

void main(){
    gl_Position = vec4(aPos, 0.0, 1.0);
    laCouleur = vec4(aRgb, 1.0);
}
