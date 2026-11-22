#pragma once

#include <glbinding/gl/gl.h>
#include <glm/glm.hpp>

#include "model.hpp"

class PhongShading;

class Windmill
{
public:
    Windmill();

    void loadModels();

    void update(float deltaTime);

    void draw(glm::mat4 &projView, glm::mat4 &view);

    // Repère du mât principal (qui porte la meule), en coordonnées monde.
    glm::mat4 getMainBeamMatrix() const;

private:
    glm::mat4 getBaseMatrix() const;

    // La matrice parente passée par copie
    void drawRoofAndRotor(glm::mat4 &projView, glm::mat4 &view, glm::mat4 baseMat);
    void drawBlades(glm::mat4 &projView, glm::mat4 &view, glm::mat4 rotorCenter);
    void drawMechanism(glm::mat4 &projView, glm::mat4 &view);

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

    PhongShading* phongShadingShader;
};
