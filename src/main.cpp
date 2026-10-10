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

struct App : public OpenGLApplication
{
    App()
        : isDay_(true), cameraPosition_(0.f, 0.f, 0.f), cameraOrientation_(0.f, 0.f), currentScene_(0), isMouseMotionEnabled_(false)
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

        // Partie 1

        // Création des shaders program (compilation et liaison).
        edgeEffectShader_.create();
        phongShadingShader_.create();
        skyShader_.create();

        windmill_.phongShadingShader = &phongShadingShader_;

        // Chargement  différentes textures
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

        CHECK_GL_ERROR;
    }

    // Appelée à chaque trame. Le buffer swap est fait juste après.
    void drawFrame() override
    {
        CHECK_GL_ERROR;
        // TODO: Partie 2: Ajouter le nettoyage du tampon de stencil
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui::Begin("Scene Parameters");
        ImGui::Combo("Scene", &currentScene_, SCENE_NAMES, N_SCENE_NAMES);

        // Et oui, il est désormais possible de recharger les shaders en gardant l'application ouvert.
        if (ImGui::Button("Reload Shaders"))
        {
            CHECK_GL_ERROR;
            edgeEffectShader_.reload();
            phongShadingShader_.reload();
            skyShader_.reload();

            setLightingUniform();
            CHECK_GL_ERROR;
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

    // TODO: À modifier, ajouter les textures, et l'effet de contour.
    void drawFences(glm::mat4 &projView, glm::mat4 &view)
    {
        const glm::vec3 FENCES_POSITIONS[] =
            {
                // TODO: Ajouter vos positions de clôture ici.
                //       Devrait permettre de mettre une cloture qui entoure le moulin.
                //       _______
                //       |     |
                //       |  M  |
                //       | | | |
                //       |_| |_|
        };

        const float FENCES_ANGLES[] =
            {
                // TODO: Ajouter vos angles de clôture ici.
        };

        // TODO: À ajouter et compléter.
        //       Dessiner les clôtures. Celles-ci ont une texture transparente,
        //       il est donc nécessaire d'activer le mélange des couleurs (GL_BLEND).
        //       De plus, vous devez dessiner les clôtures du plus loin vers le plus proche
        //       pour éviter les problèmes de mélange.
        //       Utiliser un map avec la distance en clef pour les trier (les maps trient
        //       à l'insertion).
        //       Les clôtures doivent être visibles des deux sens.
        //       Il est important de restaurer l'état du contexte qui a été modifié à la fin de la méthode.

        constexpr unsigned int N_FENCES = 0;
        std::map<float, unsigned int> sorted;
        for (unsigned int i = 0; i < N_FENCES; i++)
        {
            // TODO: Calcul de la distance par rapport à l'observateur (utiliser la matrice de vue!)
            //       et faite une insertion dans le map
        }

        // TODO: Itération à l'inverse (de la plus grande distance jusqu'à la plus petit)
        for (std::map<float, unsigned int>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
        {
            // TODO: Dessin des clôtures

            // TODO: Partie 2, pour l'effet de contour, il faut agrandir l'objet à partir de la
            //       base du model (origine placé à y=0.5, sur le haut de la clôture)
        }
    }

    // TODO: À modifier, ajouter les textures
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

    glm::mat4 getViewMatrix()
    {
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
            for (unsigned int i = 0; i < N_SPOTLIGHTS; i++)
            {
                lightsData_.spotLights[i].ambient = glm::vec4(glm::vec3(0.02f), 0.0f);
                lightsData_.spotLights[i].diffuse = glm::vec4(glm::vec3(0.8f), 0.0f);
                lightsData_.spotLights[i].specular = glm::vec4(glm::vec3(0.4f), 0.0f);
            }
        }
    }

    void setMaterial(Material &mat)
    {
        material_.updateData(&mat, 0, sizeof(Material));
    }

    // TODO: À ajouter et modifier.
    //       Ajouter les textures, les skyboxes, etc.
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
        ImGui::End();

        updateCameraInput();
        windmill_.update(deltaTime_);

        glm::mat4 view = getViewMatrix();

        // Produit projection * vue calculé UNE SEULE FOIS par trame, puis réutilisé
        glm::mat4 projView = projectionMatrix_ * view;

        phongShadingShader_.use();
        setMaterial(defaultMat);

        windmillTexture_.use();
        windmill_.draw(projView, view);

        drawGround(projView, view);

        // Penser à votre ordre de dessin, les todos sont volontairement mélangés ici.
        // Dessin du skybox
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

        // TODO: Dessin des clôtures

        // TODO: Dessin des spotlights
    }

private:
    // Shaders
    EdgeEffect edgeEffectShader_;
    PhongShading phongShadingShader_;
    Sky skyShader_;

    // Textures
    Texture2D grassTexture_;
    Texture2D fenceTexture_;
    Texture2D windmillTexture_;
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

    static constexpr unsigned int N_SPOTLIGHTS = 3;

    // Imgui var
    const char *const SCENE_NAMES[1] = {
        "Main scene"};
    const int N_SCENE_NAMES = sizeof(SCENE_NAMES) / sizeof(SCENE_NAMES[0]);
    int currentScene_;

    bool isMouseMotionEnabled_;
    glm::mat4 projectionMatrix_;
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
