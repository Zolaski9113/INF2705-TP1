#include "windmill.hpp"

#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


using namespace gl;
using namespace glm;

    
Windmill::Windmill()
: windSpeed(0.f)
, windAngle(0.f)
, angularSpeed(0.0f)
, rotorAngle(0.f)
, roofAngle(0.0f)
{}

void Windmill::loadModels()
{
    blade_.load("../models/windmill-blade.ply");
    bladeframe_.load("../models/windmill-bladeframe.ply");
    bladebeam_.load("../models/windmill-bladebeam.ply");
    mainbeam_.load("../models/windmill-mainbeam.ply");
    millstone_.load("../models/windmill-millstone.ply");
    roof_.load("../models/windmill-roof.ply");
    walls_.load("../models/windmill-walls.ply");
}

void Windmill::update(float deltaTime)
{
    if (deltaTime < 0.001)
        return;
    
    const float RADIUS = 12.5f;
    const float TSR = 6.0f;
    float angularAccel = TSR / RADIUS * windSpeed * deltaTime;

    const float FRICTION_FACTOR = 0.2f;
    float friction = angularSpeed * FRICTION_FACTOR;
    angularAccel -= friction;
    angularSpeed += angularAccel * deltaTime;
    rotorAngle += angularSpeed * deltaTime;
    
    roofAngle += TSR / RADIUS * (windAngle - roofAngle) * angularSpeed * deltaTime;
}

void Windmill::draw(glm::mat4& projView)
{
    // TODO: Dessin de la totalité du moulin.
    //
    // Caractéristique du moulin:
    // Il doit être positionné devant la vue de 10 unités et être sur le sol (origine du modèle initial
    // à 0.06).
    // Le modèle total doit être 5 fois plus gros.
    //
    // Le toit est positionné à 3.03 de hauteur.
    // Celui-ci peut tourner selon l'angle du vent (utiliser roofAngle, mise à jour dans `update`).
    //
    // Une pale complète est composé d'une pale et d'un cadre. Si on considère l'origine de la pale
    // étant le bout où elle est attaché, le centre du cadre est à 2.38 unités du bout, alors que la pale
    // est à 2.75 unités du bout et décaler de 1.23 unités.
    //
    // Les pales sont fixés sur le mât du rotor à 0.13 unités par rapport au centre de celui-ci.
    // Le modèle des pales est trop malheureusement gros: il faut un modèle 2 fois plus petit.
    //
    // La roue de pales est placé à 0.5 unité le long du mât du rotor.
    // À son tour, le mât du rotor est 0.7 unité ressorti du toit, puis 0.25 unité plus haut.
    //
    // Le mât principal est au centre du moulin.
    // Il tourne à un facteur 5 fois plus vite que la vitesse du rotor.
    //
    // La meule suit le bas du mât principal en étant fixé à 0.48 unité sur le côté et 0.15 unité au dessus
    // de la base du mât.
    // Celle-ci tourne en même temps qu'elle avance, à un rythme 2.27 fois plus rapide que la rotation
    // du mât.

    glm::mat4 base = glm::mat4(1.0f);
    base = glm::translate(base, glm::vec3(0.0f, -0.04f, -10.0f));
    base = glm::scale(base, glm::vec3(5.0f));
    base = glm::translate(base, glm::vec3(0.0f, 0.06f, 0.0f));

    glm::mat4 mvp = projView * base;
    glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, glm::value_ptr(mvp));
    walls_.draw();

    drawRoofAndRotor(projView, base);
    drawMechanism(projView, base);

}

void Windmill::drawRoofAndRotor(glm::mat4& projView, glm::mat4 baseMat)
{
    baseMat = glm::translate(baseMat, glm::vec3(0.0f, 3.03f, 0.0f));
    baseMat = glm::rotate(baseMat, roofAngle, glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 mvp = projView * baseMat;
    glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, glm::value_ptr(mvp));
    roof_.draw();

    glm::mat4 bladeBeam = glm::translate(baseMat, glm::vec3(0.0f, 0.25f, 0.7f));
    mvp = projView * bladeBeam;
    glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, glm::value_ptr(mvp));
    bladebeam_.draw();

    glm::mat4 rotorCenter = glm::translate(bladeBeam, glm::vec3(0.0f, 0.0f, 0.5f));
    rotorCenter = glm::rotate(rotorCenter, rotorAngle, glm::vec3(0.0f, 0.0f, 1.0f));

    drawBlades(projView, rotorCenter);
}

void Windmill::drawBlades(glm::mat4& projView, glm::mat4 rotorCenter)
{
    for (int i = 0; i < 4; ++i)
    {
        glm::mat4 bladeRoot = glm::rotate(rotorCenter, glm::radians(90.0f * i), glm::vec3(0.0f, 0.0f, 1.0f));
        bladeRoot = glm::translate(bladeRoot, glm::vec3(0.0f, 0.13f, 0.0f));
        glm::mat4 scaledRoot = glm::scale(bladeRoot, glm::vec3(0.5f));

        glm::mat4 frameMat = glm::translate(scaledRoot, glm::vec3(0.0f, 2.38f, 0.0f));
        frameMat = glm::rotate(frameMat, glm::radians(90.0f), glm::vec3(90.0f, 0.0f, 1.0f));

        glm::mat4 mvpFrame = projView * frameMat;
        glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, glm::value_ptr(mvpFrame));
        bladeframe_.draw();

        glm::mat4 bladeMat = glm::translate(scaledRoot, glm::vec3(-1.23f, 2.75f, 0.0f));
        bladeMat = glm::rotate(bladeMat, glm::radians(-90.0f), glm::vec3(90.0f, 0.0f, 1.0f));

        glm::mat4 mvpBlade = projView * bladeMat;
        glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, glm::value_ptr(mvpBlade));
        blade_.draw();
    }
}

void Windmill::drawMechanism(glm::mat4& projView, glm::mat4 baseMat)
{
    glm::mat4 mainBeam = glm::rotate(baseMat, rotorAngle * 5.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 mvp = projView * mainBeam;
    glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, glm::value_ptr(mvp));
    mainbeam_.draw();

    glm::mat4 millStone = glm::translate(mainBeam, glm::vec3(-0.48f, 0.15f, 0.0f));
    millStone = glm::rotate(millStone, rotorAngle * 5.0f * 2.27f, glm::vec3(1.0f, 0.0f, 0.0f));
    mvp = projView * millStone;
    glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, glm::value_ptr(mvp));
    millstone_.draw();
}
    

