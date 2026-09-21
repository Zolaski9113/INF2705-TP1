#version 330 core

// TODO: La couleur des fragments est donnée à partir de la couleur
//       des vertices passée en entrée.


in vec4 laCouleur;
out vec4 fragColor;
  
void main(void) { 
    fragColor = laCouleur; 
}