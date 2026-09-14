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

}
    

