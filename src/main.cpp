#include <cstddef>
#include <cstdint>

#include <array>
#include <cmath>
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

#include <imgui/imgui.h>

#include <inf2705/OpenGLApplication.hpp>

#include "model.hpp"
#include "model_data.hpp"
#include "shader_storage_buffer.hpp"
#include "shaders.hpp"
#include "textures.hpp"
#include "uniform_buffer.hpp"
#include "windmill.hpp"

#define CHECK_GL_ERROR printGLError(__FILE__, __LINE__)

using namespace gl;
using namespace glm;

// Définition des structures pour la communication avec le shader. NE PAS MODIFIER.

struct Material
{
    glm::vec4 emission; // vec3, but padded
    glm::vec4 ambient;  // vec3, but padded
    glm::vec4 diffuse;  // vec3, but padded
    glm::vec3 specular;
    GLfloat shininess;
};

// Partie 3 : particule, même disposition mémoire que dans le compute shader. NE PAS MODIFIER.
struct Particle
{
    glm::vec3 position;
    GLfloat zOrientation;
    glm::vec4 velocity; // vec3, but padded
    glm::vec4 color;
    glm::vec2 size;
    GLfloat timeToLive;
    GLfloat maxTimeToLive;
};

struct DirectionalLight
{
    glm::vec4 ambient;   // vec3, but padded
    glm::vec4 diffuse;   // vec3, but padded
    glm::vec4 specular;  // vec3, but padded
    glm::vec4 direction; // vec3, but padded
};

struct SpotLight
{
    glm::vec4 ambient;  // vec3, but padded
    glm::vec4 diffuse;  // vec3, but padded
    glm::vec4 specular; // vec3, but padded

    glm::vec4 position; // vec3, but padded
    glm::vec3 direction;
    GLfloat exponent;
    GLfloat openingAngle;

    GLfloat padding[3];
};

// Matériels

Material defaultMat =
    {
        {0.0f, 0.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 1.0f, 0.0f},
        {1.0f, 1.0f, 1.0f, 0.0f},
        {0.7f, 0.7f, 0.7f},
        10.0f};

Material grassMat =
    {
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.8f, 0.8f, 0.8f, 0.0f},
        {1.0f, 1.0f, 1.0f, 0.0f},
        {0.05f, 0.05f, 0.05f},
        100.0f};

// Partie 1 : spline de Bézier

Material bezierMat =
    {
        {1.0f, 1.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        1.0f};

struct BezierCurve
{
    glm::vec3 p0;
    glm::vec3 c0;
    glm::vec3 c1;
    glm::vec3 p1;
};

BezierCurve curves[5] =
    {
        {
            glm::vec3(-0.509, 2.403, 6.924),
            glm::vec3(2.412, 12.011, 20.396),
            glm::vec3(-23.731, 5.855, 25.199),
            glm::vec3(-25.732, 15.439, 13.561),
        },
        {
            glm::vec3(-25.732, 15.439, 13.561),
            glm::vec3(-27.069, 21.845, 5.781),
            glm::vec3(-4.807, 26.461, -10.716),
            glm::vec3(-7.574, 25.967, -12.861),
        },
        {
            glm::vec3(-7.574, 25.967, -12.861),
            glm::vec3(-21.299, 23.516, -23.495),
            glm::vec3(-18.956, 13.960, -36.813),
            glm::vec3(-3.324, 11.588, -33.743),
        },
        {
            glm::vec3(-3.324, 11.588, -33.743),
            glm::vec3(4.812, 10.354, -32.145),
            glm::vec3(8.408, 22.100, -7.555),
            glm::vec3(23.264, 6.067, -5.370),
        },
        {
            glm::vec3(23.264, 6.067, -5.370),
            glm::vec3(29.367, -0.518, -4.473),
            glm::vec3(2.412, 12.011, 20.396),
            glm::vec3(-0.088, 12.011, 17.896),
        },
};

struct App : public OpenGLApplication
{
    App()
        : isDay_(true), cameraPosition_(0.f, 0.f, 0.f), cameraOrientation_(0.f, 0.f), currentScene_(0), isMouseMotionEnabled_(false), totalTime(0.0), timerParticles_(0.0), nParticles_(0)
    {
    }

    void init() override
    {
        // Le message expliquant les touches de clavier.
        setKeybindMessage(
            "ESC : quitter l'application."
            "\n"
            "T : changer de scène."
            "\n"
            "W : déplacer la caméra vers l'avant."
            "\n"
            "S : déplacer la caméra vers l'arrière."
            "\n"
            "A : déplacer la caméra vers la gauche."
            "\n"
            "D : déplacer la caméra vers la droite."
            "\n"
            "Q : déplacer la caméra vers le bas."
            "\n"
            "E : déplacer la caméra vers le haut."
            "\n"
            "Flèches : tourner la caméra."
            "\n"
            "Souris : tourner la caméra"
            "\n"
            "Espace : activer/désactiver la souris."
            "\n");

        // Config de base.

        // Initialisation de la couleur de fond.
        glClearColor(0.05f, 0.1f, 0.1f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_PROGRAM_POINT_SIZE); // pour être en mesure de modifier gl_PointSize dans les shaders

        // Partie 1

        // Création des shaders program (compilation et liaison).
        edgeEffectShader_.create();
        phongShadingShader_.create();
        skyShader_.create();
        bezierShader_.create();

        // TODO: Partie 1-2-3 : créer les nouveaux shaders (courbe, herbe, dessin des particules, mise à jour des particules)

        windmill_.phongShadingShader = &phongShadingShader_;

        // Chargement des différentes textures
        grassTexture_.load("../textures/grass.jpg");
        grassTexture_.setFiltering(GL_LINEAR);
        grassTexture_.setWrap(GL_REPEAT);
        grassTexture_.enableMipmap();

        fenceTexture_.load("../textures/fence.png");
        fenceTexture_.setFiltering(GL_NEAREST);
        fenceTexture_.setWrap(GL_REPEAT);

        windmillTexture_.load("../textures/windmill.png");
        windmillTexture_.setFiltering(GL_LINEAR);
        windmillTexture_.setWrap(GL_CLAMP_TO_EDGE);
        windmillTexture_.enableMipmap();

        whiteTexture_.load("../textures/white.png");
        whiteTexture_.setFiltering(GL_NEAREST);

        // Chargement des textures des skybox
        const char *pathes[] = {
            "../textures/skybox/Daylight Box_Right.bmp",
            "../textures/skybox/Daylight Box_Left.bmp",
            "../textures/skybox/Daylight Box_Top.bmp",
            "../textures/skybox/Daylight Box_Bottom.bmp",
            "../textures/skybox/Daylight Box_Front.bmp",
            "../textures/skybox/Daylight Box_Back.bmp",
        };

        const char *nightPathes[] = {
            "../textures/skyboxNight/right.png",
            "../textures/skyboxNight/left.png",
            "../textures/skyboxNight/top.png",
            "../textures/skyboxNight/bottom.png",
            "../textures/skyboxNight/front.png",
            "../textures/skyboxNight/back.png",
        };

        skyboxTexture_.load(pathes);
        skyboxNightTexture_.load(nightPathes);

        loadModels();

        // Partie 3

        material_.allocate(&defaultMat, sizeof(Material));
        material_.setBindingIndex(0);

        lightsData_.dirLight =
            {
                {0.2f, 0.2f, 0.2f, 0.0f},
                {1.0f, 1.0f, 1.0f, 0.0f},
                {0.5f, 0.5f, 0.5f, 0.0f},
                {0.5f, -1.0f, 0.5f, 0.0f}};

        // Initialisation des paramètres de lumière

        lightsData_.spotLights[0].position = glm::vec4(-1.6, 0.64, -0.45, 0.0f);
        lightsData_.spotLights[0].direction = glm::vec3(-10, -1, 0);
        lightsData_.spotLights[0].exponent = 4.0f;
        lightsData_.spotLights[0].openingAngle = 30.f;

        lightsData_.spotLights[1].position = glm::vec4(-1.6, 0.64, 0.45, 0.0f);
        lightsData_.spotLights[1].direction = glm::vec3(-10, -1, 0);
        lightsData_.spotLights[1].exponent = 4.0f;
        lightsData_.spotLights[1].openingAngle = 30.f;

        lightsData_.spotLights[2].position = glm::vec4(1.6, 0.64, -0.45, 0.0f);
        lightsData_.spotLights[2].direction = glm::vec3(10, -1, 0);
        lightsData_.spotLights[2].exponent = 4.0f;
        lightsData_.spotLights[2].openingAngle = 60.f;

        toggleSpotlights();

        setLightingUniform();

        lights_.allocate(&lightsData_, sizeof(lightsData_));
        lights_.setBindingIndex(1);

        // Calculé uniquement la première fois et aux redimentionnements
        projectionMatrix_ = getPerspectiveProjectionMatrix();

        glGenVertexArrays(1, &vaoBezier_);
        glBindVertexArray(vaoBezier_);

        glGenBuffers(1, &vboBezier_);
        glBindBuffer(GL_ARRAY_BUFFER, vboBezier_);
        glBufferData(GL_ARRAY_BUFFER, N_CURVES * (MAX_BEZIER_EXTRA_POINTS + 2) * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        // TODO: Partie 2 : générer le mesh des patches triangulaires qui couvre le sol (VAO, VBO, EBO)

        // TODO: Partie 3 : allouer les deux SSBO pour MAX_PARTICLES_ particules.
        //       Seul le buffer en entrée est initialisé à 0. Réfléchir au type d'usage.

        // TODO: Partie 3 : créer le VAO pour le dessin des particules et activer les attributs nécessaires

        // TODO: Partie 3 : charger la texture des particules, "../textures/smoke.png"
    }

    // Appelée à chaque trame. Le buffer swap est fait juste après.
    void drawFrame() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        ImGui::Begin("Scene Parameters");
        ImGui::Combo("Scene", &currentScene_, SCENE_NAMES, N_SCENE_NAMES);

        // Recharge les shaders sans fermer l'application.
        if (ImGui::Button("Reload Shaders"))
        {
            edgeEffectShader_.reload();
            phongShadingShader_.reload();
            skyShader_.reload();
            bezierShader_.reload();
            setLightingUniform();
        }
        ImGui::End();

        switch (currentScene_)
        {
        case 0:
            sceneMain();
            break;
        }
        CHECK_GL_ERROR;
    }

    // Appelée lors d'une touche de clavier.
    void onKeyPress(const sf::Event::KeyPressed &key) override
    {
        using enum sf::Keyboard::Key;
        switch (key.code)
        {
        case Escape:
            window_.close();
            break;
        case Space:
            isMouseMotionEnabled_ = !isMouseMotionEnabled_;
            if (isMouseMotionEnabled_)
            {
                window_.setMouseCursorGrabbed(true);
                window_.setMouseCursorVisible(false);
            }
            else
            {
                window_.setMouseCursorGrabbed(false);
                window_.setMouseCursorVisible(true);
            }
            break;
        case T:
            currentScene_ = ++currentScene_ < N_SCENE_NAMES ? currentScene_ : 0;
            break;
        default:
            break;
        }
    }

    void onResize(const sf::Event::Resized &event) override
    {
        projectionMatrix_ = getPerspectiveProjectionMatrix();
    }

    void onMouseMove(const sf::Event::MouseMoved &mouseDelta) override
    {
        if (!isMouseMotionEnabled_)
            return;

        const float MOUSE_SENSITIVITY = 0.1;
        float cameraMouvementX = mouseDelta.position.y * MOUSE_SENSITIVITY;
        float cameraMouvementY = mouseDelta.position.x * MOUSE_SENSITIVITY;
        cameraOrientation_.y -= cameraMouvementY * deltaTime_;
        cameraOrientation_.x -= cameraMouvementX * deltaTime_;
    }

    void updateCameraInput()
    {
        if (!window_.hasFocus())
            return;

        if (isMouseMotionEnabled_)
        {
            sf::Vector2u windowSize = window_.getSize();
            sf::Vector2i windowHalfSize(windowSize.x / 2.0f, windowSize.y / 2.0f);
            sf::Mouse::setPosition(windowHalfSize, window_);
        }

        float cameraMouvementX = 0;
        float cameraMouvementY = 0;

        const float KEYBOARD_MOUSE_SENSITIVITY = 1.5f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            cameraMouvementX -= KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            cameraMouvementX += KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            cameraMouvementY -= KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            cameraMouvementY += KEYBOARD_MOUSE_SENSITIVITY;

        cameraOrientation_.y -= cameraMouvementY * deltaTime_;
        cameraOrientation_.x -= cameraMouvementX * deltaTime_;

        // Keyboard input
        glm::vec3 positionOffset = glm::vec3(0.0);
        const float SPEED = 10.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            positionOffset.z -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            positionOffset.z += SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            positionOffset.x -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            positionOffset.x += SPEED;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q))
            positionOffset.y -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
            positionOffset.y += SPEED;

        positionOffset = glm::rotate(glm::mat4(1.0f), cameraOrientation_.y, glm::vec3(0.0, 1.0, 0.0)) * glm::vec4(positionOffset, 1);
        cameraPosition_ += positionOffset * glm::vec3(deltaTime_);
    }

    void loadModels()
    {
        windmill_.loadModels();
        fence_.load("../models/fence.ply");
        spotlight_.load("../models/spotlight.ply");
        skybox_.load("../models/skybox.ply");

        grass_.load(ground, sizeof(ground), planeElements, sizeof(planeElements));
    }

    void drawFences(glm::mat4 &projView, glm::mat4 &view)
    {
        const glm::vec3 FENCES_POSITIONS[] =
            {
                // Mur arrière (Z fixé à -17.5)
                glm::vec3(-5.0f, 0.0f, -17.5f), glm::vec3(0.0f, 0.0f, -17.5f), glm::vec3(5.0f, 0.0f, -17.5f),

                // Mur gauche (4 clôtures s'allongeant vers l'avant)
                glm::vec3(-7.5f, 0.0f, -15.0f), glm::vec3(-7.5f, 0.0f, -10.0f),
                glm::vec3(-7.5f, 0.0f, -5.0f), glm::vec3(-7.5f, 0.0f, 0.0f),

                // Mur droit (4 clôtures s'allongeant vers l'avant)
                glm::vec3(7.5f, 0.0f, -15.0f), glm::vec3(7.5f, 0.0f, -10.0f),
                glm::vec3(7.5f, 0.0f, -5.0f), glm::vec3(7.5f, 0.0f, 0.0f),

                // Mur avant (Z = 2.5), ouvert au centre pour l'entrée
                glm::vec3(-5.0f, 0.0f, 2.5f), glm::vec3(5.0f, 0.0f, 2.5f),

                // Couloir d'entrée, de part et d'autre de l'ouverture
                glm::vec3(-2.5f, 0.0f, 0.0f), glm::vec3(2.5f, 0.0f, 0.0f)};

        const float FENCES_ANGLES[] =
            {
                // Arrière
                glm::radians(90.0f), glm::radians(90.0f), glm::radians(90.0f),
                // Gauche
                0.0f, 0.0f, 0.0f, 0.0f,
                // Droite
                0.0f, 0.0f, 0.0f, 0.0f,
                // Avant
                glm::radians(90.0f), glm::radians(90.0f),
                // Couloir d'entrée
                0.0f, 0.0f

        };

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_STENCIL_TEST);
        glDisable(GL_CULL_FACE);

        fenceTexture_.use();

        constexpr unsigned int N_FENCES = 15;
        // multimap : deux clôtures à la même distance (enclos symétrique) ne s'écrasent pas
        std::multimap<float, unsigned int> sorted;
        for (unsigned int i = 0; i < N_FENCES; i++)
        {
            // Calcul de la distance par rapport à l'observateur et insertion dans le map
            glm::vec4 posInView = view * glm::vec4(FENCES_POSITIONS[i], 1.0f);
            float distance = glm::length(glm::vec3(posInView));
            sorted.insert({distance, i});
        }

        for (std::multimap<float, unsigned int>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
        {
            unsigned int index = it->second;

            // — Dessiner l’objet en couleur, mais aussi dans le tampon de stencil lorsque seulement le test de profondeur passe.
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            glStencilMask(0xFF);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, FENCES_POSITIONS[index]);
            model = glm::rotate(model, FENCES_ANGLES[index], glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(5.0f));

            glm::mat4 mvp = projView * model;
            phongShadingShader_.setMatrices(mvp, view, model);

            fence_.draw();

            // On change le teste pour ne rien écrire dans le tampon de stencil.
            glStencilMask(0x00);

            // On change le teste pour réussir lorsque le tampon de stencil n’a pas eu d’écriture précédemment.
            glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
            // — On dessine une seconde fois le même objet, mais avec un autre shader tout en agrandissant l’objet.
            edgeEffectShader_.use();

            glm::mat4 edgeModel = glm::scale(model, glm::vec3(1.05f));
            glm::mat4 edgeMvp = projView * edgeModel;

            glUniformMatrix4fv(edgeEffectShader_.mvpULoc, 1, GL_FALSE, glm::value_ptr(edgeMvp));
            fence_.draw();

            // On remet le shader normal pour la prochaine itération de la boucle
            phongShadingShader_.use();
        }
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glDisable(GL_STENCIL_TEST);

        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
    }

    void drawGround(glm::mat4 &projView, glm::mat4 &view)
    {
        // Carré unitaire agrandi à 50 x 50 et abaissé de 0.1.
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.1f, 0.0f));
        model = glm::scale(model, glm::vec3(50.0f, 1.0f, 50.0f));

        glm::mat4 mvp = projView * model;

        setMaterial(grassMat);
        phongShadingShader_.setMatrices(mvp, view, model);
        grassTexture_.use();
        grass_.draw();
    }

    void drawSpotlights(glm::mat4 &projView, glm::mat4 &view)
    {
        whiteTexture_.use();

        for (unsigned int i = 0; i < N_SPOTLIGHTS; i++)
        {
            Material spotMat = defaultMat;
            spotMat.ambient = glm::vec4(0.0f);
            spotMat.diffuse = glm::vec4(0.0f);
            spotMat.specular = glm::vec3(0.0f);
            spotMat.emission = lightsData_.spotLights[i].diffuse;
            setMaterial(spotMat);

            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(lightsData_.spotLights[i].position));
            glm::mat4 mvp = projView * model;
            phongShadingShader_.setMatrices(mvp, view, model);
            spotlight_.draw();
        }

        setMaterial(defaultMat);
    }

    glm::vec3 evaluateBezier(const BezierCurve &curve, float t)
    {
        // B(t) = (1-t)³·p0 + 3(1-t)²·t·c0 + 3(1-t)·t²·c1 + t³·p1

        float u = 1.0f - t;

        return (u * u * u * curve.p0) + (3 * u * u * t * curve.c0) + (3 * u * t * t * curve.c1) + (t * t * t * curve.p1);
    }

    void drawBezier(glm::mat4 &projView)
    {
        bezierShader_.use();
        glUniformMatrix4fv(bezierShader_.mvpULoc, 1, GL_FALSE, glm::value_ptr(projView));

        glBindVertexArray(vaoBezier_);
        for (unsigned int c = 0; c < N_CURVES; c++)
            glDrawArrays(GL_LINE_STRIP, c * (bezierNPoints + 2), bezierNPoints + 2);
    }

    glm::mat4 getViewMatrix()
    {
        if (cameraMode == 1)
            return glm::lookAt(cameraPosition_, glm::vec3(0.0f, 2.0f, -10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        // La vue est l'inverse de la caméra
        glm::mat4 view = glm::mat4(1.0f);

        view = glm::rotate(view, -cameraOrientation_.x, glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, -cameraOrientation_.y, glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::translate(view, -cameraPosition_);

        return view;
    }

    glm::mat4 getPerspectiveProjectionMatrix()
    {
        return glm::perspective(glm::radians(70.0f), getWindowAspect(), 0.1f, 300.0f);
    }

    void setLightingUniform()
    {
        phongShadingShader_.use();
        glUniform1i(phongShadingShader_.nSpotLightsULoc, 3);

        float ambientIntensity = 0.05;
        glUniform3f(phongShadingShader_.globalAmbientULoc, ambientIntensity, ambientIntensity, ambientIntensity);
    }

    void toggleSun()
    {
        if (isDay_)
        {
            lightsData_.dirLight.ambient = glm::vec4(0.2f, 0.2f, 0.2f, 0.0f);
            lightsData_.dirLight.diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
            lightsData_.dirLight.specular = glm::vec4(0.5f, 0.5f, 0.5f, 0.0f);
        }
        else
        {
            lightsData_.dirLight.ambient = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
            lightsData_.dirLight.diffuse = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
            lightsData_.dirLight.specular = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
        }
    }

    void toggleSpotlights()
    {
        if (isDay_)
        {
            for (unsigned int i = 0; i < N_SPOTLIGHTS; i++)
            {
                lightsData_.spotLights[i].ambient = glm::vec4(glm::vec3(0.0f), 0.0f);
                lightsData_.spotLights[i].diffuse = glm::vec4(glm::vec3(0.0f), 0.0f);
                lightsData_.spotLights[i].specular = glm::vec4(glm::vec3(0.0f), 0.0f);
            }
        }
        else
        {
            {
                const glm::vec3 SPOT_COLORS[N_SPOTLIGHTS] = {
                    glm::vec3(1.0f, 0.0f, 0.0f), // rouge
                    glm::vec3(0.0f, 0.0f, 1.0f), // bleu
                    glm::vec3(1.0f, 1.0f, 1.0f)  // blanc
                };

                for (unsigned int i = 0; i < N_SPOTLIGHTS; i++)
                {
                    lightsData_.spotLights[i].ambient = glm::vec4(SPOT_COLORS[i] * 0.02f, 0.0f);
                    lightsData_.spotLights[i].diffuse = glm::vec4(SPOT_COLORS[i] * 0.8f, 0.0f);
                    lightsData_.spotLights[i].specular = glm::vec4(SPOT_COLORS[i] * 0.4f, 0.0f);
                }
            }
        }
    }

    void setMaterial(Material &mat)
    {
        material_.updateData(&mat, 0, sizeof(Material));
    }

    void sceneMain()
    {
        ImGui::Begin("Scene Parameters");
        if (ImGui::Button("Toggle Day/Night"))
        {
            isDay_ = !isDay_;
            toggleSun();
            toggleSpotlights();
            lights_.updateData(&lightsData_, 0, sizeof(DirectionalLight) + N_SPOTLIGHTS * sizeof(SpotLight));
        }
        ImGui::SliderFloat("Wind Speed", &windmill_.windSpeed, 0.0f, 20.0f, "%.2f m/s");
        ImGui::SliderFloat("Wind Angle", &windmill_.windAngle, -M_PI, M_PI, "%.2f°");
        ImGui::SliderInt("Bezier Number Of Points", (int *)&bezierNPoints, 0, MAX_BEZIER_EXTRA_POINTS);
        if (ImGui::Button("Animate Camera"))
        {
            isAnimatingCamera = true;
            cameraMode = 1;
        }
        ImGui::End();

        if (!isAnimatingCamera)
            updateCameraInput();

        windmill_.update(deltaTime_);

        if (isAnimatingCamera)
        {
            if (cameraAnimation < 5)
            {
                unsigned int index = static_cast<unsigned int>(cameraAnimation); // partie entière de cameraAnimation
                float t = cameraAnimation - index; // partie fractionnaire
                cameraPosition_ = evaluateBezier(curves[index], t);

                cameraAnimation += deltaTime_ / 3.0;
            }
            else
            {
                cameraPosition_ = curves[4].p1;

                // Remise à 0 de l'orientation
                glm::vec3 distance = glm::vec3(0, 2, -10) - cameraPosition_;

                float horizontalDistance = sqrt(distance.x * distance.x + distance.z * distance.z);

                cameraOrientation_.y = atan2(-distance.x, -distance.z);
                cameraOrientation_.x = atan2(distance.y, horizontalDistance);

                cameraAnimation = 0.f;
                isAnimatingCamera = false;
                cameraMode = 0;
            }
        }

        bool hasNumberOfSidesChanged = bezierNPoints != oldBezierNPoints;
        if (hasNumberOfSidesChanged)
        {
            oldBezierNPoints = bezierNPoints;

            std::vector<glm::vec3> points;

            for (const BezierCurve &curve : curves)
                for (unsigned int i = 0; i <= bezierNPoints + 1; i++)
                {
                    float t = float(i) / (bezierNPoints + 1);
                    points.push_back(evaluateBezier(curve, t));
                }

            glBindBuffer(GL_ARRAY_BUFFER, vboBezier_);
            glBufferSubData(GL_ARRAY_BUFFER, 0, points.size() * sizeof(glm::vec3), points.data());
        }

        glm::mat4 view = getViewMatrix();

        // Produit projection * vue calculé UNE SEULE FOIS par trame, puis réutilisé
        glm::mat4 projView = projectionMatrix_ * view;

        // Objets opaques d'abord
        phongShadingShader_.use();
        setMaterial(defaultMat);

        windmillTexture_.use();
        windmill_.draw(projView, view);

        drawGround(projView, view);

        drawSpotlights(projView, view);

        drawBezier(projView);

        // TODO: Partie 2 : dessiner l'herbe (grassMat)

        // Ciel après les opaques : il ne remplit que les pixels encore vides
        skyShader_.use();

        glm::mat4 skyMvp = projectionMatrix_ * glm::mat4(glm::mat3(view));
        glUniformMatrix4fv(skyShader_.mvpULoc, 1, GL_FALSE, glm::value_ptr(skyMvp));

        if (isDay_)
            skyboxTexture_.use();
        else
            skyboxNightTexture_.use();

        glDepthFunc(GL_LEQUAL);
        skybox_.draw();
        glDepthFunc(GL_LESS);
        phongShadingShader_.use();

        // Clôtures transparentes en dernier, pour se mélanger avec ce qui est derrière
        drawFences(projView, view);

        // Partie 3 : particules, dessinées après les objets opaques car leur texture est transparente.
        totalTime += deltaTime_;
        timerParticles_ += deltaTime_;
        const float particlesSpawnInterval = 0.2f;

        unsigned int particlesToAdd = timerParticles_ / particlesSpawnInterval;
        timerParticles_ -= particlesToAdd * particlesSpawnInterval;

        nParticles_ += particlesToAdd;
        if (nParticles_ > MAX_PARTICLES_)
            nParticles_ = MAX_PARTICLES_;

        // TODO: Partie 3 : mise à jour des particules avec le compute shader.
        //       Envoyer les uniforms, configurer les buffers d'entrée et de sortie, puis glDispatchCompute.
        //       Utiliser la position de la meule pour la position et la direction d'émission.
        // glm::vec3 emitterPos = glm::vec3(0.0, 0.0, 0.0);
        // glm::vec3 emitterDir = glm::vec3(1, 0, 0);

        // TODO: Partie 3 : dessin des particules (nParticles_ particules, buffer en entrée comme source d'attributs).
        //       Activer le blending, désactiver l'écriture de profondeur, puis restaurer l'état.

        // TODO: Partie 3 : interchanger les buffers d'entrée et de sortie
    }

    void onClose() override
    {
        glDeleteVertexArrays(1, &vaoBezier_);
        glDeleteBuffers(1, &vboBezier_);
    }

private:
    // Shaders
    EdgeEffect edgeEffectShader_;
    PhongShading phongShadingShader_;
    Sky skyShader_;
    Bezier bezierShader_;

    // Textures
    Texture2D grassTexture_;
    Texture2D fenceTexture_;
    Texture2D windmillTexture_;
    Texture2D whiteTexture_;
    TextureCubeMap skyboxTexture_;
    TextureCubeMap skyboxNightTexture_;

    // Uniform buffers
    UniformBuffer material_;
    UniformBuffer lights_;

    struct
    {
        DirectionalLight dirLight;
        SpotLight spotLights[8];
    } lightsData_;

    bool isDay_;

    Model grass_;
    Model fence_;
    Model spotlight_;
    Model skybox_;

    Windmill windmill_;

    glm::vec3 cameraPosition_;
    glm::vec2 cameraOrientation_;

    // Partie 1 : spline de Bézier
    static constexpr unsigned int N_CURVES = 5;
    static constexpr unsigned int MAX_BEZIER_EXTRA_POINTS = 16;
    unsigned int bezierNPoints = 3; // Nombre de points supplémentaires sur la courbe, 0 est une ligne droite
    unsigned int oldBezierNPoints = 0;
    GLuint vaoBezier_;
    GLuint vboBezier_;

    int cameraMode = 0;
    float cameraAnimation = 0.f;
    bool isAnimatingCamera = false;

    // TODO: Partie 1-2 : ajouter les attributs vao, vbo, ebo nécessaires (spline et patches d'herbe)

    static constexpr unsigned int N_SPOTLIGHTS = 3;

    // Imgui var
    const char *const SCENE_NAMES[1] = {
        "Main scene"};
    const int N_SCENE_NAMES = sizeof(SCENE_NAMES) / sizeof(SCENE_NAMES[0]);
    int currentScene_;

    bool isMouseMotionEnabled_;
    glm::mat4 projectionMatrix_;

    // Partie 3 : particules
    float totalTime;
    float timerParticles_;

    static const unsigned int MAX_PARTICLES_ = 64;
    unsigned int nParticles_;

    GLuint vaoParticles_;
    ShaderStorageBuffer particles_[2];

    // TODO: Partie 1-2-3 : ajouter les shaders (courbe, herbe, dessin et mise à jour des particules)
    // TODO: Partie 3 : ajouter la texture des particules
};

int main(int argc, char *argv[])
{
    WindowSettings settings = {};
    settings.fps = 60;
    settings.context.depthBits = 24;
    settings.context.stencilBits = 8;
    settings.context.antiAliasingLevel = 4;
    settings.context.majorVersion = 3;
    settings.context.minorVersion = 3;
    settings.context.attributeFlags = sf::ContextSettings::Attribute::Core;

    App app;
    app.run(argc, argv, "Tp2", settings);
}
