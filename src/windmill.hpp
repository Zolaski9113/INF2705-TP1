#pragma once

#include <glbinding/gl/gl.h>
#include <glm/glm.hpp>

#include "model.hpp"

class Windmill
{   
public:
    Windmill();
    
    void loadModels();
    
    void update(float deltaTime);
    
    void draw(glm::mat4& projView);
    
private:
    // TODO: À vous de faire une division en méthodes pour le dessin des différentes parties.
    //       Il sera important de penser à comment réutiliser le plus possible les matrices.
    //       Utilisez le passage par référence ou par copie pour vous aider. Faites des attributs
    //       supplémentaire si nécessaire.
    void drawRoofAndRotor(glm::mat4& projView, glm::mat4 baseMat);
    void drawBlades(glm::mat4& projView, glm::mat4 rotorCenter);
    void drawMechanism(glm::mat4& projView, glm::mat4 baseMat);

    
private:    
    Model blade_;
    Model bladeframe_;
    Model bladebeam_;
    Model mainbeam_;
    Model millstone_;
    Model roof_;
    Model walls_;
    
public:
    float windSpeed;
    float windAngle;

    float angularSpeed;
    float rotorAngle;
    
    float roofAngle;
    
    GLuint mvpUniformLocation;
};


