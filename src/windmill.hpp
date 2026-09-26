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

    void draw(glm::mat4 &projView);

private:
    // La matrice parente passée par copie
    void drawRoofAndRotor(glm::mat4 &projView, glm::mat4 baseMat);
    void drawBlades(glm::mat4 &projView, glm::mat4 rotorCenter);
    void drawMechanism(glm::mat4 &projView, glm::mat4 baseMat);

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
